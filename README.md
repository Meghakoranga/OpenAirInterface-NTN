# NTN Configuration Guide: GEO and LEO Configurations

This guide outlines setting up GEO and LEO configurations for the NTN (Non-Terrestrial Network) project using OpenAirInterface (OAI). It covers gNB and UE details, configuration files, commands, and log management.
```bash
git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git ~/openairinterface5g
cd ~/openairinterface5g
```


## gnb Configuration Files
 
**GEO Configuration:**
- Path: `ci-scripts/conf_files/gnb.sa.band254.u0.25prb.rfsim.ntn.conf`

**LEO Configuration:**
- Path: `ci-scripts/conf_files/gnb.sa.band254.u0.25prb.rfsim.ntn-leo.conf`

## gNB Configuration Details
The following changes are to be made in  `ci-scripts/conf_files/gnb.sa.band254.u0.25prb.rfsim.ntn.conf` 
### Key Parameters

**channel model** : rfsimulator has to be configured to apply the channel model.
This can be done by providing this line in the conf file in section rfsimulator:

  ```options = "chanmod";```
## Simulation Parameters
### cellSpecificKoffset_r17
```bash
 cellSpecificKoffset_r17=478;                              #GEO
 cellSpecificKoffset_r17=40;                                #LEO
```
### HARQ
```bash
    disable_harq = 1; //    #GEO
    num_dlharq = 32;        #LEO
    num_ulharq = 32;        #LEO


```
##
### GEO Simulation
Add the following to gNB and UE command lines:
```bash
--rfsimulator.prop_delay 238.74
```

### LEO Simulation
Two models:
- **SAT_LEO_TRANS**: transparent satellite, gNB on ground
- **SAT_LEO_REGEN**: regenerative satellite, gNB on board

Example for transparent LEO:
```bash
channelmod = {
  max_chan=10;
  modellist="modellist_rfsimu_1";
  modellist_rfsimu_1 = (
    {
      model_name     = "rfsimu_channel_enB0"
      type           = "SAT_LEO_TRANS";
      noise_power_dB = -100;
    },
    {
      model_name     = "rfsimu_channel_ue0"
      type           = "SAT_LEO_TRANS";
      noise_power_dB = -100;
    }
  );
};
```
Add to conf file for leo under `rfsimulator`:
```bash
options = ("chanmod");
```
To simulate a LEO satellite channel model with rfsimulator in UL (DL is simulated at the UE side), either the channelmod section as shown before has to be added to the gNB conf file, or a channelmod conf file has to be included like this:
```bash
@include "channelmod_rfsimu_LEO_satellite.conf"
```

## Commands
After configuring the necessary configuration files, you can run the following commands:


 **GEO GNB :**
```bash
cd cmake_targets
sudo ./ran_build/build/nr-softmodem -O ../ci-scripts/conf_files/gnb.sa.band254.u0.25prb.rfsim.ntn.conf --rfsim --rfsimulator.prop_delay 238.74> gnb_geo.log 2>&1
```
```> gnb_geo.log 2>&1```  is used to include options to redirect both standard output and errors to log 
**GEO UE**
```bash
cd cmake_targets
sudo ./ran_build/build/nr-uesoftmodem -O ../targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf --band 254 -C 2488400000 --CO -873500000 -r 25 --numerology 0 --ssb 60 --rfsim --rfsimulator.prop_delay 238.74> ue_geo.log 2>&1
```


 **LEO GNB  :**
```bash
cd cmake_targets
sudo ./ran_build/build/nr-softmodem -O ../ci-scripts/conf_files/gnb.sa.band254.u0.25prb.rfsim.ntn-leo.conf --rfsim > gnb_leo.log 2>&1
```


 **LEO UE  :**
```bash
cd cmake_targets
sudo ./ran_build/build/nr-uesoftmodem -O ../targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf --band 254 -C 2488400000 --CO -873500000 -r 25 --numerology 0 --ssb 60 --rfsim --rfsimulator.prop_delay 20 --rfsimulator.options chanmod --time-sync-I 0.1 --ntn-initial-time-drift -46 --autonomous-ta> ue_leo.log 2>&1
```



