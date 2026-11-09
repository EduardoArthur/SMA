# ⚙️ Core Mechanics - Workflow Instructions

## Overview

This workflow guides you through defining the mechanical foundation of your game - the ACTIONS players will perform repeatedly. By the end, you'll have comprehensive documentation of your core and secondary mechanics.

**Time:** 60-90 minutes  
**Difficulty:** Intermediate  
**Output:** `mechanics/core-mechanics.md`, `mechanics/secondary-mechanics.md`

---

## Step 1: Define the Core Verb

### The Foundation of Your Game

Every game has ONE primary action that defines what it IS.

#### Examples

| Game | Core Verb | Why It Works |
|------|-----------|--------------|
| Mario | Jump | Satisfying arc, versatile (combat, navigation) |
| Doom | Shoot | Visceral, immediate, varied (weapons) |
| Tetris | Rotate/Place | Simple, infinite depth |
| Dark Souls | Time (attacks/dodges) | High skill ceiling, meaningful |
| Stardew | Plant/Harvest | Relaxing cycle, clear progress |

#### Define Your Core Verb

```yaml
Core Mechanic:
  Verb: [The action in one word]
  Full Description: [What exactly happens]
  
  Frequency:
    Per Second: [If applicable]
    Per Minute: [Typical rate]
    Per Session: [Rough count]
  
  Why It's Satisfying:
    Visual: [What player sees]
    Audio: [What player hears]
    Tactile: [Controller/input feel]
    Result: [What changes in game]
  
  Skill Expression:
    Beginner: [How novices use it]
    Expert: [How masters use it]
    Difference: [What separates them]
```

---

## Step 2: Secondary Mechanics

### Adding Depth Without Complexity

Secondary mechanics MODIFY or EXTEND the core verb.

#### Relationship Types

| Type | Description | Example |
|------|-------------|---------|
| **Enhancer** | Makes core better | Double Jump |
| **Modifier** | Changes how core works | Charged Shot |
| **Contextual** | Core in new situation | Wall Jump |
| **Alternate** | Different approach to core | Stealth vs Combat |
| **Combo** | Chains with core | Jump + Attack |

#### Secondary Mechanics Template

```yaml
Secondary Mechanic: [Name]
  
  Relation to Core: [Enhancer/Modifier/Contextual/Alternate/Combo]
  
  Description:
    What It Does: [Action description]
    When Available: [Always / Unlocked / Contextual]
    Input: [How to perform]
  
  Purpose:
    Player Expression: [How it enables creativity]
    Strategic Value: [When it's optimal]
    Feel: [What makes it satisfying]
  
  Balancing Factors:
    Cost: [Resource / Cooldown / Risk]
    Reward: [Benefit gained]
    Trade-off: [What you give up]
```

List 3-7 secondary mechanics that support your core.

---

## Step 3: Mechanic Interactions

### Making Systems Talk

The magic happens when mechanics COMBINE.

#### Interaction Matrix

Create a grid showing how mechanics interact:

```
              | Core  | Sec A | Sec B | Sec C |
--------------|-------|-------|-------|-------|
Core          |   -   |       |       |       |
Secondary A   |       |   -   |       |       |
Secondary B   |       |       |   -   |       |
Secondary C   |       |       |       |   -   |
```

Fill each cell with:
- **✓** = Designed synergy
- **E** = Emergent (discovered)
- **X** = Intentionally blocked
- **-** = No interaction

#### Interaction Documentation

For each meaningful interaction:

```yaml
Interaction: [Name]
  
  Mechanics Involved:
    - [Mechanic A]
    - [Mechanic B]
  
  Result: [What happens]
  
  Discovery:
    Type: [Designed / Emergent / Accident]
    Tutorial: [Is this taught?]
    Required: [Must player use this?]
  
  Balance:
    Power Level: [Weak / Moderate / Strong / OP]
    Skill Required: [Low / Medium / High]
```

---

## Step 4: Input Mapping

### From Intention to Action

Every mechanic needs a clear, responsive input.

#### Controller Mapping

```yaml
Primary Layout: [Controller Type]

Face Buttons:
  A/Cross: 
    Action: [What it does]
    Hold Behavior: [If different when held]
    
  B/Circle:
    Action: [What it does]
    Context: [If contextual]
    
  X/Square:
    Action: [What it does]
    
  Y/Triangle:
    Action: [What it does]

Shoulders:
  LB/L1: [Action]
  RB/R1: [Action]
  LT/L2: [Action - often analog]
  RT/R2: [Action - often analog]

Sticks:
  Left: [Movement/Selection]
  Right: [Camera/Aim]
  L3 Click: [Action]
  R3 Click: [Action]

D-Pad:
  Up: [Quick action/item]
  Down: [Quick action/item]
  Left: [Quick action/item]
  Right: [Quick action/item]
```

#### Input Guidelines

| Principle | Guideline | Example |
|-----------|-----------|---------|
| Frequency | Most used = best button | Jump on A/X |
| Safety | Dangerous = harder to hit | Ultimate on L1+R1 |
| Grouping | Related actions near each other | All combat on right side |
| Consistency | Same input = same result | Confirm always on A |

---

## Step 5: Action Feedback

### Confirming Every Input

Players need INSTANT confirmation that their action worked.

#### Feedback Layers

```yaml
Mechanic: [Name]

Visual Feedback:
  Character: [Animation played]
  Effects: [VFX/particles]
  UI: [Any UI response]
  World: [Environmental reaction]

Audio Feedback:
  Sound Effect: [What plays]
  Variation: [Different sounds?]
  Spatial: [3D positioned?]

Haptic Feedback:
  Vibration: [Pattern/intensity]
  Trigger Resistance: [If PS5/Xbox Series]

Timing:
  Input to Response: [ms]
  Animation Duration: [ms]
  Recovery Time: [ms before next action]
```

#### Feedback Checklist

For each mechanic, ensure:
- [ ] Visual confirmation within 50ms
- [ ] Audio confirmation within 50ms
- [ ] Clear success/failure indication
- [ ] Appropriate "weight" to the action
- [ ] Consistent feedback every time

---

## Document Output

### File Structure

```
_gdks-output/02-design/core-design/mechanics/
├── core-mechanics.md
│   ├── Core Verb Definition
│   ├── Why It Works
│   └── Skill Expression
│
├── secondary-mechanics.md
│   ├── Mechanic List
│   ├── Relationships to Core
│   └── Unlock Progression
│
├── interactions.md
│   ├── Interaction Matrix
│   └── Key Combos/Synergies
│
└── input-mapping.md
    ├── Controller Layout
    ├── Keyboard Layout
    └── Feedback Specifications
```

---

## Next Steps

After defining mechanics:

1. **Design Loops** → `@core-loop-specialist` with `*loops`
   - How mechanics fit into the gameplay loop

2. **Test Feel** → `@game-feel-physicist` with `*feel`
   - Polish the sensation of each action

3. **Plan Progression** → `@progression-architect` with `*progression`
   - How mechanics unlock over time

4. **Validate Agency** → `@player-agency-advocate` with `*agency`
   - Ensure meaningful choices

---

## Tips for Great Mechanics

✅ **DO:**
- Start with ONE core verb and perfect it
- Make the basic action feel amazing
- Test mechanics with placeholder art
- Let secondary mechanics emerge from play
- Ensure every input has clear feedback

❌ **DON'T:**
- Add mechanics to "have more features"
- Copy mechanics without understanding WHY they work
- Design secondary before core is solid
- Ignore input response times
- Skip feedback (visual, audio, haptic)

---

## Let's Begin!

**What is the ONE action players will do most in your game?**

I'm Marco, and I'm here to make your mechanics feel incredible! ⚙️
