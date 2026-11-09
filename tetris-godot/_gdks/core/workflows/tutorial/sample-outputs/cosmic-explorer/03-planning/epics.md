# Cosmic Explorer — Epics

*Sample. Sprint 5 tutorial.*

## Epic 1: Drift Mechanic
**Priority:** Must
**Target sprint:** 1

### Stories
- **1.1** Character drifts with momentum-based control
- **1.2** Character can anchor to tagged surfaces
- **1.3** Anchor transition feels responsive (< 300 ms)
- **1.4** Drift respects drag coefficient from design

### Dependencies
None (foundation).

---

## Epic 2: Resonance Pulse
**Priority:** Must
**Target sprint:** 2

### Stories
- **2.1** Player emits one pulse per room
- **2.2** Pulse reveals hidden paths visually (shader pass)
- **2.3** Pulse has tuned audio feedback
- **2.4** Pulse counter resets on room transition

### Dependencies
- Epic 1 (drift must exist first)

---

## Epic 3: Level 1 — Airlock
**Priority:** Must
**Target sprint:** 3

### Stories
- **3.1** Airlock layout and geometry
- **3.2** Tutorial-integrated drift intro
- **3.3** First anchor surface teaches the mechanic
- **3.4** First narrative fragment (environmental)

### Dependencies
- Epic 1, Epic 2

---

## Epic 4: Level Flow
**Priority:** Should
**Target sprint:** 4+

### Stories
- **4.1** Room-to-room transition system
- **4.2** Save / resume at room boundary
- **4.3** Ambient score system (layered)
