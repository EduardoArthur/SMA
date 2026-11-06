# Cosmic Explorer — UE5 Architecture

*Sample. Sprint 5 tutorial.*

## Class Hierarchy

```
AActor
└── APawn
    └── ACharacter
        └── ACosmicCharacter                     (C++)

UActorComponent
└── UCharacterMovementComponent
    └── UDriftMovementComponent                  (C++)

UActorComponent
└── UResonancePulseComponent                     (C++)

AActor
└── AAnchorSurface                               (Blueprint)
    ├── BP_AnchorSurface_Wall
    ├── BP_AnchorSurface_Floor
    └── BP_AnchorSurface_Ceiling
```

## C++ vs Blueprint Boundary

- **C++:** Movement math, pulse timing, input handling, save logic.
  Anything numeric-heavy or called every frame.
- **Blueprint:** Room setup, visual effects, anchor surface
  variants, narrative props. Anything designers tune.

## Subsystems

- **`UCosmicGameInstanceSubsystem`** — tracks current room, player
  save state, pulse count.
- **`UAmbientAudioSubsystem`** — layered ambient score management,
  crossfades between rooms.

## Key Tunables (UPROPERTY)

```cpp
// UDriftMovementComponent
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift")
float BaseSpeed = 1.5f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift")
float MaxSpeed = 3.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift")
float DragCoefficient = 0.15f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift")
float AnchorTransitionTime = 0.3f;
```

## Profiling Targets

- 60 FPS on mid-range 2020+ GPU
- < 200 MB RAM for runtime state
- No garbage-collection hitches during drift (prefer structs)
