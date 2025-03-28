/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file common/utils/load_module_shlib.c
 * \brief shared library loader implementation
 * \author Francois TABURET
 * \date 2017
 * \version 0.1
 * \company NOKIA BellLabs France
 * \email: francois.taburet@nokia-bell-labs.com
 * \note
 * \warning
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include "openair1/PHY/defs_common.h"
#include "common/oai_version.h"
#define LOAD_MODULE_SHLIB_MAIN

#include "common/config/config_userapi.h"
#include "load_module_shlib.h"
loader_data_t loader_data;

void loader_init(void) {
  paramdef_t LoaderParams[] = LOADER_PARAMS_DESC;

  loader_data.mainexec_buildversion = OAI_PACKAGE_VERSION;
  int ret = config_get(config_get_if(), LoaderParams, sizeofArray(LoaderParams), LOADER_CONFIG_PREFIX);
  if (ret <0) {
       fprintf(stderr, "[LOADER]  configuration couldn't be performed via config module, parameters set to default values\n");
       if (loader_data.shlibpath == NULL) {
         loader_data.shlibpath=DEFAULT_PATH;
        }
       loader_data.maxshlibs = DEFAULT_MAXSHLIBS;
  }
  loader_data.shlibs = malloc(loader_data.maxshlibs * sizeof(loader_shlibdesc_t));
  if(loader_data.shlibs == NULL) {
     fprintf(stderr,"[LOADER]  %s %d memory allocation error %s\n",__FILE__, __LINE__,strerror(errno));
     exit_fun("[LOADER] unrecoverable error");
  }
  memset(loader_data.shlibs,0,loader_data.maxshlibs * sizeof(loader_shlibdesc_t));
}

/* build the full shared lib name from the module name */
static char *loader_format_shlibpath(char *modname, char *version)
{
  char *tmpstr;
  char *shlibpath = NULL;
  char *shlibversion = NULL;
  // clang-format off
  paramdef_t LoaderParams[] = {
    {"shlibpath",    NULL, 0, .strptr = &shlibpath,    .defstrval = NULL, TYPE_STRING, 0, NULL},
    {"shlibversion", NULL, 0, .strptr = &shlibversion, .defstrval = "",   TYPE_STRING, 0, NULL}
  };
  // clang-format on

  int ret;

  /* looks for specific path for this module in the config file */
  /* specific value for a module path and version is located in a modname subsection of the loader section */
  /* shared lib name is formatted as lib<module name><module version>.so */
  char cfgprefix[sizeof(LOADER_CONFIG_PREFIX)+strlen(modname)+16];
  sprintf(cfgprefix,LOADER_CONFIG_PREFIX ".%s",modname);
  ret = config_get(config_get_if(), LoaderParams, sizeofArray(LoaderParams), cfgprefix);
  if (ret <0) {
    fprintf(stderr, "[LOADER]  %s %d couldn't retrieve config from section %s\n", __FILE__, __LINE__, cfgprefix);
  }
  /* no specific path, use loader default shared lib path */
  if (shlibpath == NULL) {
    shlibpath = loader_data.shlibpath;
  }
  /* no specific shared lib version */
  if (version != NULL) { // version specified as a function parameter
    shlibversion = version;
  }
  if (shlibversion == NULL) { // no specific version specified, neither as a config param or as a function param
    shlibversion = "";
  }
  /* alloc memory for full module shared lib file name */
  tmpstr = malloc(strlen(shlibpath) + strlen(modname) + strlen(shlibversion) + 16);
  if (tmpstr == NULL) {
    fprintf(stderr, "[LOADER] %s %d malloc error loading module %s, %s\n", __FILE__, __LINE__, modname, strerror(errno));
    exit_fun("[LOADER] unrecoverable error");
  }
  if (shlibpath[0] != 0) {
    ret = sprintf(tmpstr, "%s/", shlibpath);
  } else {
    ret = 0;
  }

  sprintf(tmpstr + ret, "lib%s%s.so", modname, shlibversion);

  return tmpstr;
}

int load_module_version_shlib(char *modname, char *version, loader_shlibfunc_t *farray, int numf, void *autoinit_arg)
{
  void *lib_handle = NULL;
  initfunc_t fpi;
  checkverfunc_t fpc;
  getfarrayfunc_t fpg;
  char *shlib_path = NULL;
  char *afname = NULL;
  int ret = 0;
  int lib_idx = -1;

  if (!modname) {
    fprintf(stderr, "[LOADER] load_module_shlib(): no library name given\n");
    return -1;
  }

  if (!loader_data.shlibpath) {
     loader_init();
  }

  shlib_path = loader_format_shlibpath(modname, version);

  for (int i = 0; i < loader_data.numshlibs; i++) {
    if (strcmp(loader_data.shlibs[i].name, modname) == 0) {
      lib_idx = i;
      break;
    }
  }
  if (lib_idx < 0) {
    lib_idx = loader_data.numshlibs;
    ++loader_data.numshlibs;
    if (loader_data.numshlibs > loader_data.maxshlibs) {
      fprintf(stderr, "[LOADER] can not load more than %u shlibs\n", loader_data.maxshlibs);
      ret = -1;
      goto load_module_shlib_exit;
    }
    loader_data.shlibs[lib_idx].name = strdup(modname);
    loader_data.shlibs[lib_idx].thisshlib_path = strdup(shlib_path);
  }

  lib_handle = dlopen(shlib_path, RTLD_LAZY|RTLD_NODELETE|RTLD_GLOBAL);
  if (!lib_handle) {
    fprintf(stderr,"[LOADER] library %s is not loaded: %s\n", shlib_path,dlerror());
    ret = -1;
    goto load_module_shlib_exit;
  }

  afname = malloc(strlen(modname)+15);
  if (!afname) {
    fprintf(stderr, "[LOADER] unable to allocate memory for library %s\n", shlib_path);
    ret = -1;
    goto load_module_shlib_exit;
  }
  sprintf(afname,"%s_checkbuildver",modname);
  fpc = dlsym(lib_handle,afname);
  if (fpc) {
    int chkver_ret = fpc(loader_data.mainexec_buildversion,
                         &(loader_data.shlibs[lib_idx].shlib_buildversion));
    if (chkver_ret < 0) {
      fprintf(stderr, "[LOADER]  %s %d lib %s, version mismatch",
              __FILE__, __LINE__, modname);
      ret = -1;
      goto load_module_shlib_exit;
    }
  }
  sprintf(afname,"%s_autoinit",modname);
  fpi = dlsym(lib_handle,afname);

  if (fpi) {
    fpi(autoinit_arg);
  }

  if (farray) {
    loader_shlibdesc_t *shlib = &loader_data.shlibs[lib_idx];
    if (!shlib->funcarray) {
      shlib->funcarray = calloc(numf, sizeof(loader_shlibfunc_t));
      if (!shlib->funcarray) {
        fprintf(stderr, "[LOADER] load_module_shlib(): unable to allocate memory\n");
        ret = -1;
        goto load_module_shlib_exit;
      }
      shlib->len_funcarray = numf;
      shlib->numfunc = 0;
    }
    for (int i = 0; i < numf; i++) {
      farray[i].fptr = dlsym(lib_handle,farray[i].fname);
      if (!farray[i].fptr) {
        fprintf(stderr, "[LOADER] load_module_shlib(): function %s not found: %s\n",
                  farray[i].fname, dlerror());
        ret = -1;
        goto load_module_shlib_exit;
      }
      /* check whether this function has been loaded before */
      int j = 0;
      for (; j < shlib->numfunc; ++j) {
        if (shlib->funcarray[j].fptr == farray[i].fptr) {
          int rc = strcmp(shlib->funcarray[j].fname, farray[i].fname);
          AssertFatal(rc == 0,
                      "reloading the same fptr with different fnames (%s, %s)\n",
                      shlib->funcarray[i].fname, farray[i].fname);
          break;
        }
      }
      if (j == shlib->numfunc) {
        if (shlib->numfunc == shlib->len_funcarray) {
          loader_shlibfunc_t *n = realloc(shlib->funcarray, shlib->numfunc * 2 * sizeof(loader_shlibfunc_t));
          if (!n) {
            fprintf(stderr, "[LOADER] %s(): unable to allocate memory\n", __func__);
            ret = -1;
            goto load_module_shlib_exit;
          }
          shlib->funcarray = n;
          shlib->len_funcarray = shlib->numfunc * 2;
        }
        shlib->funcarray[j].fname = strdup(farray[i].fname);
        shlib->funcarray[j].fptr = farray[i].fptr;
        shlib->numfunc++;
      }
    } /* for int i... */
  } else {  /* farray ! NULL */
    sprintf(afname,"%s_getfarray",modname);
    fpg = dlsym(lib_handle,afname);
    if (fpg) {
      loader_data.shlibs[lib_idx].numfunc =
          fpg(&(loader_data.shlibs[lib_idx].funcarray));
    }
  } /* farray ! NULL */

load_module_shlib_exit:
  if (shlib_path) free(shlib_path);
  if (afname)     free(afname);
  if (lib_handle) dlclose(lib_handle);
  return ret;
}

void * get_shlibmodule_fptr(char *modname, char *fname)
{
    for (int i=0; i<loader_data.numshlibs && loader_data.shlibs[i].name != NULL; i++) {
        if ( strcmp(loader_data.shlibs[i].name, modname) == 0) {
            for (int j =0; j<loader_data.shlibs[i].numfunc ; j++) {
                 if (strcmp(loader_data.shlibs[i].funcarray[j].fname, fname) == 0) {
                     return loader_data.shlibs[i].funcarray[j].fptr;
                 }
            } /* for j loop on module functions*/
        }
    } /* for i loop on modules */
    return NULL;
}

void loader_reset()
{
  for (int i = 0; i < loader_data.numshlibs && loader_data.shlibs[i].name != NULL; i++) {
    loader_shlibdesc_t *shlib = &loader_data.shlibs[i];
    free(shlib->name);
    free(shlib->thisshlib_path);
    for (int j = 0; j < shlib->numfunc; ++j)
      free(shlib->funcarray[j].fname);
    free(shlib->funcarray);
    shlib->numfunc = 0;
    shlib->len_funcarray = 0;
  }
  if(loader_data.shlibpath){
    free(loader_data.shlibpath);
    loader_data.shlibpath=NULL;
  }
  free(loader_data.shlibs);
}
