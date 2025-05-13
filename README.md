# T-37 Flight Simulator – Senior Design II

Welcome to the GitHub repository for the **T-37 Dual-Cockpit Flight Simulator**, a senior capstone project developed for the **Tulsa Air and Space Museum (TASM)** in collaboration with **CymSTAR**. This repository contains all relevant software, documentation, CAD drawings, and project deliverables from the **Senior Design II** course at The University of Tulsa.

---

## 🚀 Project Overview

The T-37 Flight Simulator is a multiplayer, interactive museum exhibit designed to replicate the experience of piloting a T-37 aircraft. Built using **X-Plane 12** and a custom C++ plugin, the simulator allows two users to cooperatively control a virtual aircraft from a restored T-37 cockpit.

### 🔧 Key Features

- Dual flight sticks using **BNO055 orientation sensors** with **additive control logic** via Arduino Leonardo
- **Real-time gameplay loop** with orb collection and a 90-second countdown
- **LED lighting** effects and **Buttkicker haptic feedback** for immersive experience
- **Mounted 1080p displays**, arcade-style UI, and automatic game start/recovery
- Validated with live user testing at TASM, including children and adults

---

## 📁 Repository Structure

T_37_Flight_Simulator/

├── Distribution/ # Final plugin binaries and resource files

│ ├── 01_XplaneBatch.bat

│ ├── Cessna T-37 Situation.sit

│ ├── T37_plugin/ # Compiled plugin (win.xpl)

│ └── T37_Resources/ # Textures, HUDs, 3D models, scoreboard

│
├── Documentation/

│ ├── Drawing/ # Full CAD drawings and schematics

│ ├── Final Reports/

│ ├── Maintence Documentation/

│ ├── Requirements/

│ ├── Test Plan and Report/

│ ├── User Manual/

│ └── Weekly Reports/

│
├── Source Code/ # Custom C++ plugin source code

│ ├── T37_plugin.cpp

│ ├── DataRefs.cpp

│ └── stb_image.h

│

├── .gitattributes

├── README.md

---

## 📄 Final Deliverables

- ✅ **[Final Report (PDF)](./Documentation/Final%20Reports/T37_FinalReport_V3_Signed.pdf)** – Full documentation of design, testing, and results  
- ✅ **[Maintenance Manual (PDF)](./Documentation/Maintence%20Documentation/T37_MaintenanceManual_V3.pdf)** – For museum staff on upkeep and troubleshooting  
- ✅ **[User Manual (PDF)](./Documentation/User%20Manual/T37_UserManual_V1.pdf)** – Guide to using the simulator as an exhibit  
- ✅ **[Test Plan & Report](./Documentation/Test%20Plan%20and%20Report/T37_TestingReport_V1_Signed.pdf)** – Safety and gameplay validation  
- ✅ **CAD Drawings** – Found in `/Documentation/Drawing`, includes joystick, seat, HUD, wiring, and block diagrams  
- ✅ **Compiled Plugin + Resources** – Located in `/Distribution`, used with X-Plane 12  
- ✅ **Source Code** – Full C++ source under `/Source Code/` for plugin development  

---

## 👨‍💻 Team Members

- **Zachariah Chorette** – Project Lead  
- **Mary Claire Giessen** – Testing Engineer  
- **Luke Ozment** – Software Engineer  
- **Saul Aguayo** – Manufacturing Engineer

---

## 🏛️ Partner Institutions

- **Tulsa Air and Space Museum (TASM)** – Project customer and exhibit provider  
- **CymSTAR** – Provided engineering guidance and review  

---

## 🧪 Testing & Validation

The simulator was validated through:

- Multiplayer input accuracy (additive joystick logic)
- Thermal benchmarking (FurMark & Cinebench stress tests)
- Live user testing (children as young as six)
- 10+ hour continuous durability evaluation
- Electrical and mechanical safety per **OSHA Title 29 §1910**

All system-level and customer-defined requirements were verified and are documented in the [Final Report](./Documentation/Final%20Reports/T37_FinalReport_V3_Signed.pdf).

---

## 📌 Repository Notes

This repository is intended to provide future maintainers and collaborators with:
- Access to full design documentation
- Plugin development references
- Distribution-ready exhibit software

For questions, please reach out to [Zachariah Chorette](mailto:zachariah-chorette@utulsa.edu).
