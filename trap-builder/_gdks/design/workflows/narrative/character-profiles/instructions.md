# 👤 Character Profiles - Workflow Instructions

## Overview

This workflow guides you through creating detailed character profiles for all major characters in your game. By the end, you'll have comprehensive documentation that captures who your characters ARE, not just what they do.

**Time:** 30-60 minutes per character  
**Difficulty:** Intermediate  
**Output:** `characters/[name].md` for each character

---

## Step 1: Character Basics

### The Foundation

Let's establish the fundamental facts about your character.

```yaml
Character Sheet:
  Name: [Full name]
  Nickname/Alias: [If any]
  Role: [Protagonist/Antagonist/Ally/NPC]
  
  Demographics:
    Age: [Number or range]
    Gender: [Gender identity]
    Species/Race: [If applicable]
    
  Background:
    Origin: [Where they come from]
    Occupation: [What they do]
    Social Status: [Their position in society]
    
  First Impression:
    Visual Hook: [Most memorable visual trait]
    Personality Hook: [First thing you notice about them]
    Voice: [How they sound/speak]
```

---

## Step 2: Psychology & Motivation

### What Drives Them

The heart of a memorable character is their inner world.

#### The Want/Need Framework

```yaml
External Want: [What they consciously pursue]
  - Why: [Reason they think they want this]
  - How: [Methods they'll use]
  
Internal Need: [What they actually need to grow]
  - Blind Spot: [Why they can't see it]
  - Catalyst: [What might reveal it]
  
The Ghost: [Past trauma or formative event]
  - What Happened: [The event]
  - Impact: [How it shaped them]
  - Wound: [Resulting emotional damage]
  
The Lie: [False belief they hold about themselves/world]
  - Statement: [The lie in their words]
  - Evidence: [Why they believe it]
  - Truth: [What they need to learn]
```

#### Personality Profile

```yaml
Core Traits:
  - [Trait 1]: [How it manifests]
  - [Trait 2]: [How it manifests]
  - [Trait 3]: [How it manifests]
  
Strengths:
  - [Strength]: [Example situation]
  
Flaws:
  - [Flaw]: [How it causes problems]
  
Fears:
  - Surface Fear: [What obviously scares them]
  - Deep Fear: [What they truly fear]
  
Values:
  - Will Fight For: [What they protect]
  - Won't Cross: [Their moral line]
```

---

## Step 3: Character Arc

### How They Change

Characters should be different at the end than they were at the beginning.

#### Arc Template

```yaml
Arc Type: [Positive/Negative/Flat/Transformative]

Beginning State:
  Who They Are: [Description]
  Belief: [What they believe]
  Behavior: [How they act]
  
Inciting Disruption:
  Event: [What shakes their world]
  Challenge to Belief: [How it questions their lie]
  
Midpoint Shift:
  Revelation: [What they learn/experience]
  New Approach: [How they change tactics]
  
Crisis Point:
  Lowest Moment: [When everything seems lost]
  Choice: [Decision they must make]
  
Resolution:
  Final State: [Who they become]
  New Belief: [What they now believe]
  Evidence: [How we see the change]
  
Arc Summary:
  "[CHARACTER] goes from [START STATE] to [END STATE] by [PROCESS]"
```

---

## Step 4: Relationships

### Connections Define Characters

No character exists in isolation. Map their key relationships.

#### Relationship Map

```yaml
Primary Relationships:
  [Character Name]:
    Type: [Family/Friend/Lover/Rival/Enemy/Mentor]
    Dynamic: [How they interact]
    History: [How they met, key events]
    Tension: [Source of conflict]
    Function: [Story purpose]
    
Secondary Relationships:
  [Character Name]:
    Type: [Role]
    One-Line: [Summarize the relationship]
```

#### Relationship Dynamics

| Character | Relationship | Dynamic | Story Function |
|-----------|--------------|---------|----------------|
| [Name] | [Type] | [How they interact] | [What it serves] |

---

## Step 5: Physical Presence

### Visual Design Direction

Provide guidance for artists and animators.

```yaml
Appearance:
  Body:
    Build: [Body type]
    Height: [Relative scale]
    Posture: [How they carry themselves]
    
  Face:
    Shape: [General face structure]
    Eyes: [Color, expression tendency]
    Distinguishing: [Scars, marks, features]
    
  Style:
    Clothing: [What they wear, why]
    Accessories: [Signature items]
    Colors: [Associated palette]
    
  Movement:
    Walk: [How they move]
    Gestures: [Common mannerisms]
    Combat/Action: [Fighting style if applicable]
```

---

## Step 6: Voice & Dialogue

### How They Speak

```yaml
Voice Profile:
  Sound: [Description of voice]
  Pace: [Fast/Slow/Variable]
  Volume: [Loud/Quiet/Varies]
  
Speech Patterns:
  Vocabulary: [Simple/Complex/Technical/Slang]
  Sentence Structure: [Short/Long/Fragmented]
  Verbal Tics: [Repeated words/phrases]
  
Sample Lines:
  Happy: "[Example dialogue]"
  Angry: "[Example dialogue]"
  Sad: "[Example dialogue]"
  Signature: "[Their most characteristic line]"
  
Things They'd Never Say:
  - "[Out of character line]"
```

---

## Document Output

Each character profile will be saved to:
```
_gdks-output/02-design/narrative/characters/[character-name].md
```

### Profile Structure

```markdown
# Character Profile: [Name]

## Quick Reference
- Role: 
- One-Line: 

## Identity
[Basics, demographics, background]

## Psychology
[Want/Need, personality, fears, values]

## Arc
[Beginning → Change → End]

## Relationships
[Key connections]

## Physical
[Appearance, movement, style]

## Voice
[Speech patterns, sample dialogue]

## Story Notes
[How they serve the narrative]
```

---

## Next Steps

After creating character profiles:

1. **Map Relationships** → `*relationships` for the full relationship web
2. **Design Arcs** → `*arcs` for detailed arc mapping
3. **Visual Design** → `@concept-artist` with `*charart` for art specs
4. **Dialogue Design** → `@narrative-designer` with `*dialogue`

---

## Tips for Great Characters

✅ **DO:**
- Give them contradictions (brave but anxious)
- Make their flaws connected to their strengths
- Let them be wrong sometimes
- Give them agency (they ACT, not just react)
- Consider how players will feel about them

❌ **DON'T:**
- Make them perfect (Mary Sue/Gary Stu)
- Define them only by their trauma
- Forget they have lives outside the plot
- Make all villains pure evil
- Neglect supporting characters

---

## Let's Begin!

**Who is your first character? Start with whoever feels most central to your story.**

I'm Charlie, and I'm excited to bring your characters to life! 👤
