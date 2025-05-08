# T-37 Flight Simulator – Senior Design II

Welcome to the GitHub repository for the **T-37 Dual-Cockpit Flight Simulator**, a senior capstone project developed for the **Tulsa Air and Space Museum (TASM)** in collaboration with **CymSTAR**. This repository contains all relevant software, documentation, CAD drawings, and deliverables from the **Senior Design II** course at The University of Tulsa.

---

## 🚀 Project Overview

The T-37 Flight Simulator is a multiplayer, interactive museum exhibit designed to replicate the experience of piloting a T-37 aircraft. Built using **X-Plane 12** and a custom C++ plugin, the simulator allows two users to cooperatively control a virtual aircraft from a single, restored T-37 cockpit.

Key features include:

- Dual flight sticks using **BNO055 orientation sensors** and **additive control logic** via Arduino Leonardo
- **Real-time gameplay loop** with orb collection and a 90-second countdown timer
- **LED lighting** and **Buttkicker haptic feedback** for enhanced immersion
- **Mounted 1080p displays**, arcade-style UI, and automatic game startup/recovery
- Tested and validated with children and adults at TASM with strong feedback

---

## 📄 Final Deliverables

- ✅ **[Final Report (PDF)](./Documentation/Final%20Reports/T37_FinalReport_V2.pdf)** – Detailed technical documentation of the system’s design, development, and testing  
- ✅ **[Maintenance Manual (PDF)](./Documentation/Maintence%20Documentation/T37_MaintenanceManual_V1.pdf)** – Setup, usage, and troubleshooting guide for museum staff  
- ✅ **CAD Drawings** – Full mechanical and wiring schematics (see `/Drawings`)  
- ✅ **Custom Plugin Source Code** – Located in `/Software/Plugin`, built with X-Plane SDK  
- ✅ **Startup Scripts & Configuration** – Found in `/Software/Startup`  

---

## 👨‍💻 Team Members

- **Zachariah Chorette** – Project Lead  
- **Mary Claire Giessen** – Testing Engineer  
- **Luke Ozment** – Software Engineer  
- **Saul Aguayo** – Manufacturing Engineer

---

## 🏛️ Partner Institutions

- **Tulsa Air and Space Museum (TASM)** – Project customer, provided cockpit shell and exhibit integration
- **CymSTAR** – Engineering consulting and design review

---

## 🧪 Testing & Validation

The simulator underwent rigorous testing, including:

- Multiplayer input validation (adder logic)
- Thermal stress testing (FurMark & Cinebench)
- Usability testing with children as young as six
- 10-hour durability test
- Safety verification per OSHA Title 29 §1910 compliance

All requirements were met as documented in the [Final Report](./Documentation/Final%20Reports/T37_FinalReport_V2.pdf).

---

