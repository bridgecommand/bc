# Ship Coefficient Database

Reference for all ownship models and their physics parameters, including MMG readiness.

**Last Updated:** 2026-02-09

## Overview

Bridge Command includes 14 ownship models. The MMG (Maneuvering Modeling Group) physics model
provides higher-fidelity ship handling than the legacy dynamics model. MMG is enabled per-ship
via `MMGMode=1` in `boat.ini` and only works with conventional (non-azimuth) propulsion.

### MMG Status Summary

| Ship | Type | Propulsion | MMG Ready | Notes |
|------|------|-----------|-----------|-------|
| ProtisSingleScrew | Cargo/Ferry | Single-screw | **Enabled** | Reference ship |
| Protis | Cargo/Ferry | Twin-screw | **Enabled** | Same hull as ProtisSingleScrew |
| VIC56 | Puffer/Cargo | Single-screw | **Enabled** | Traditional vessel |
| VIC56_360 | Puffer/Cargo | Single-screw | **Enabled** | VIC56 with 360 pano views |
| Puffer | Puffer/Cargo | Single-screw | **Enabled** | Same physics as VIC56 |
| HMAS_Westralia | Naval Tanker | Twin-screw | **Enabled** | Large vessel |
| Alkmini | Cargo | Twin-screw | **Enabled** | Large cargo vessel |
| Aquarius_Tug | Harbor Tug | Twin-screw | Candidate | High bollard pull, needs tuning |
| CSL_Wattle | Coastal Vessel | Twin-screw | Candidate | Small vessel |
| Alice Upjohn | Small Vessel | Twin-screw | Candidate | Very small, marginal |
| 3111_Tug | Tug | Azimuth | N/A | Azimuth drive, MMG incompatible |
| ShetlandTrader | Cargo/Ferry | Azimuth | N/A | Azimuth drive, MMG incompatible |
| Waverley | Paddle Steamer | Paddle | Not suitable | Unusual propulsion |
| Atlantic85 | RNLI Lifeboat | Twin outboard | Not suitable | Planing hull, not displacement |

## MMG Parameters

When `MMGMode=1`, the following boat.ini parameters are used:

| Parameter | Required | Default | Description |
|-----------|----------|---------|-------------|
| `MMGMode` | Yes | 0 | Set to 1 to enable MMG physics |
| `BlockCoefficient` | Recommended | 0.65 | Fullness of hull (0.4-0.9) |
| `maxSpeedAhead` | Recommended | ~10 m/s | Service speed in **knots** |
| `PropellerDiameter` | Optional | draught * 0.65 | Propeller diameter in metres |
| `PropellerMaxRPM` | Optional | MaxRevs or 120 | Max propeller shaft RPM |
| `MMG_mx_prime` | Optional | Estimated | Explicit MMG surge added mass |
| `MMG_my_prime` | Optional | Estimated | Explicit MMG sway added mass |
| `MMG_Jz_prime` | Optional | Estimated | Explicit MMG yaw added inertia |
| `MMG_Yv_prime` | Optional | Estimated | Sway force derivative |
| `MMG_Yr_prime` | Optional | Estimated | Sway force due to yaw rate |
| `MMG_Nv_prime` | Optional | Estimated | Yaw moment due to sway |
| `MMG_Nr_prime` | Optional | Estimated | Yaw moment derivative |
| `MMG_tP` | Optional | Estimated | Thrust deduction factor |
| `MMG_wP0` | Optional | Estimated | Wake fraction at straight ahead |

If `MMG_mx_prime` is 0 or absent, all coefficients are estimated from ship dimensions using
empirical correlations (Clarke 1983, Yoshimura 2012).

## Detailed Ship Data

### ProtisSingleScrew (Reference)
- **Type:** Cargo/Ferry, single-screw conventional
- **Draught:** 3.0 m | **Mass:** 300,000 kg | **Inertia:** 14,040,000 kg*m^2
- **Max Force:** 180,000 N | **Max Speed:** 12.0 kts
- **Block Coefficient:** 0.65
- **Propeller:** 1.8 m diameter, 300 RPM max
- **MMG Status:** Enabled (MMGMode=1) -- reference vessel for MMG validation

### Protis
- **Type:** Cargo/Ferry, twin-screw conventional
- **Draught:** 3.0 m | **Mass:** 300,000 kg | **Inertia:** 14,040,000 kg*m^2
- **Max Force:** 180,000 N/engine | **Max Speed:** 12.0 kts
- **Block Coefficient:** 0.65 (same hull as ProtisSingleScrew)
- **Propeller:** 1.5 m diameter (smaller for twin), 300 RPM max
- **MMG Status:** Enabled (MMGMode=1)
- **Notes:** Same hull form as ProtisSingleScrew, twin-screw variant

### VIC56 / VIC56_360 / Puffer
- **Type:** Clyde Puffer / small cargo, single-screw conventional
- **Draught:** 9.0 m (model scale) | **Mass:** 145,000 kg | **Inertia:** 8,000,000 kg*m^2
- **Max Force:** 14,500 N | **Max Speed:** 8.0 kts
- **Block Coefficient:** 0.75 (full-bodied cargo hull)
- **Propeller:** 1.2 m diameter (explicit, auto-estimate from draught too large), 140 RPM
- **MMG Status:** Enabled (MMGMode=1)
- **Notes:** Three variants share identical physics. VIC56_360 uses panoramic images.

### HMAS_Westralia
- **Type:** Naval tanker/replenishment ship, twin-screw conventional
- **Draught:** 26.4 m | **Mass:** 30,000,000 kg | **Inertia:** 80,000,000,000 kg*m^2
- **Max Force:** 2,145,273 N/engine | **Max Speed:** 16.0 kts
- **Block Coefficient:** 0.70 (typical for naval auxiliary)
- **Propeller:** 6.0 m diameter (explicit), 150 RPM
- **MMG Status:** Enabled (MMGMode=1)
- **Notes:** Large vessel. Real HMAS Westralia was ~167m LOA.

### Alkmini
- **Type:** Large cargo vessel, twin-screw conventional
- **Draught:** 18.0 m | **Mass:** 35,000,000 kg | **Inertia:** 87,500,000,000 kg*m^2
- **Max Force:** 2,600,000 N/engine | **Max Speed:** 14.0 kts
- **Block Coefficient:** 0.75 (typical for bulk carrier)
- **Propeller:** 6.5 m diameter (explicit), 100 RPM
- **MMG Status:** Enabled (MMGMode=1)
- **Notes:** Has bow and stern thrusters (100 kN each at 67m distance).

### Aquarius_Tug (Candidate)
- **Type:** Harbor tug, twin-screw conventional
- **Draught:** 4.0 m | **Mass:** 1,607,828 kg | **Inertia:** 148,237,264 kg*m^2
- **Max Force:** 1,024,140 N/engine | **Est. Speed:** ~12 kts
- **Block Coefficient:** Not set (est. 0.55 for tug hull)
- **Propeller:** Not set (est. 2.6 m from draught)
- **MMG Status:** Candidate -- needs tuning for high-bollard-pull operation
- **Notes:** Very high thrust-to-displacement ratio typical of tugs. DynamicsSpeedB=0 (unusual).

### CSL_Wattle (Candidate)
- **Type:** Coastal vessel, twin-screw conventional
- **Draught:** 12.0 m | **Mass:** 160,000 kg | **Inertia:** 18,100,000 kg*m^2
- **Max Force:** 42,112 N | **Est. Speed:** ~10 kts
- **Block Coefficient:** Not set (est. 0.60)
- **Propeller:** Not set
- **MMG Status:** Candidate

### Alice Upjohn (Candidate)
- **Type:** Small traditional vessel, twin-screw
- **Draught:** 2.8 m | **Mass:** 13,000 kg | **Inertia:** 150,000 kg*m^2
- **Max Force:** 1,300 N | **Est. Speed:** ~6 kts
- **Block Coefficient:** Not set
- **MMG Status:** Candidate -- very small, may need careful coefficient selection

### 3111_Tug (Azimuth Drive)
- **Type:** Tug, azimuth stern drive
- **Draught:** 4.3 m | **Mass:** 650,000 kg | **Block Coefficient:** 0.87
- **Max Speed:** 6.0 kts | **Propeller:** Not applicable (azimuth)
- **MMG Status:** N/A -- requires azimuth drive model (future work)

### ShetlandTrader (Azimuth Drive)
- **Type:** Cargo/ferry, azimuth stern drive
- **Draught:** 4.3 m | **Mass:** 650,000 kg | **Block Coefficient:** 0.87
- **Max Speed:** 5.2 kts | **Propeller:** Not applicable (azimuth)
- **MMG Status:** N/A -- requires azimuth drive model (future work)

### Waverley (Not Suitable)
- **Type:** Paddle steamer
- **Draught:** 2.07 m | **Mass:** 693,000 kg | **Inertia:** 100,000,000 kg*m^2
- **Max Force:** 106,000 N | **AsternEfficiency:** 1.0 (symmetric)
- **MMG Status:** Not suitable -- paddle propulsion not modeled by MMG
- **Notes:** No prop walk. High rudder force (RudderA=15000). Low RudderAngularVelocity=10.

### Atlantic85 (Not Suitable)
- **Type:** RNLI lifeboat, twin outboard
- **Draught:** 0.5 m | **Mass:** 1,800 kg | **Inertia:** 120,000 kg*m^2
- **Max Force:** 4,760 N | **MaxRevs:** 6000
- **MMG Status:** Not suitable -- planing hull at speed, MMG assumes displacement mode

## Estimation Notes

When `BlockCoefficient` is not specified in boat.ini, the code defaults:
- For mass estimation (Ship.cpp): uses 0.87 as typical
- For MMG model: uses 0.65 if not specified

Typical block coefficients by vessel type:
- Container ship: 0.55-0.65
- Cargo/general: 0.65-0.80
- Bulk carrier: 0.75-0.85
- Tanker: 0.80-0.87
- Tug: 0.50-0.60
- Naval vessel: 0.45-0.55
- Ferry: 0.55-0.65

`PropellerDiameter` is auto-estimated as `draught * 0.65` when not specified. This works well
for ships where the draught value is realistic but can produce unrealistic values for ships
where the "Depth" parameter in boat.ini represents something other than physical draught.
Explicitly specify PropellerDiameter when the auto-estimate would be unreasonable.
