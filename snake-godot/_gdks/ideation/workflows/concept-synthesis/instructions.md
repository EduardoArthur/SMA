# Workflow: Concept Synthesis

## Purpose
Bring together all ideation work into a cohesive concept document, ready for handoff to the Design Team.

## When to Use
- After completing brainstorming, market analysis, and mechanics exploration
- When preparing to transition to the Design phase
- To ensure nothing is lost between phases

---

## Prerequisites

Before running this workflow, you should have:

- [ ] `concept-brief.md` - Core concept and hook
- [ ] `market-analysis.md` - Market positioning and competitors
- [ ] `mechanics-exploration.md` - Core loops and systems

---

## Step 1: Gather All Materials

Review existing documents:

```
_gdks-output/01-ideation/
├── concept-brief.md          ← Core concept
├── market-analysis.md        ← Market research
└── mechanics-exploration.md  ← Gameplay mechanics
```

### Quick Check
For each document, confirm:
- [ ] Document exists and is complete
- [ ] No contradictions with other documents
- [ ] Key decisions are documented with reasoning

---

## Step 2: Identify Gaps

Review for missing information:

### Concept Completeness
- [ ] Game title (working title is fine)
- [ ] Genre and sub-genre defined
- [ ] Target platform(s) specified
- [ ] Unique hook articulated
- [ ] Core fantasy defined
- [ ] Elevator pitch written

### Market Completeness
- [ ] Target audience identified
- [ ] Competitor games analyzed
- [ ] Market positioning clear
- [ ] Price point considered
- [ ] Release window considered

### Mechanics Completeness
- [ ] Core verb identified
- [ ] Core loop defined
- [ ] Key systems listed
- [ ] Prototype priorities set
- [ ] Technical risks noted

### Fill Any Gaps Now
If anything is missing, address it before proceeding.

---

## Step 3: Resolve Contradictions

Common contradictions to check:

### Scope vs. Resources
- Does the concept match available resources?
- Are mechanics achievable with the team/time?

### Hook vs. Market
- Does the unique hook appeal to the target audience?
- Is there market demand for this type of innovation?

### Mechanics vs. Fantasy
- Do the mechanics support the core fantasy?
- Will players FEEL what you want them to feel?

### Document Contradictions Found:
1. ___________
2. ___________

### Resolutions:
1. ___________
2. ___________

---

## Step 4: Create Unified Vision Statement

Write a comprehensive vision statement (1-2 paragraphs):

### Template:
```
[GAME TITLE] is a [GENRE] game for [PLATFORM] targeting [AUDIENCE].

Players experience the fantasy of [CORE FANTASY] through [CORE MECHANICS].
The game stands out by [UNIQUE HOOK], differentiating from competitors
like [COMPETITOR 1] and [COMPETITOR 2].

The core experience is [EMOTION/FEELING], achieved through [KEY FEATURES].
Development scope is [SCOPE LEVEL] with an estimated timeline of [TIMELINE].
```

---

## Step 5: Define Success Criteria

How will you know if the game is successful?

### Creative Success
- [ ] Players understand the hook immediately
- [ ] Core loop is engaging for X hours
- [ ] Reviews mention the unique element
- [ ] Players describe the intended emotion

### Commercial Success (if applicable)
- [ ] Sales target: ___________
- [ ] Review score target: ___________
- [ ] Player count target: ___________
- [ ] Break-even point: ___________

### Personal Success
- [ ] Learn new skills
- [ ] Complete a full project
- [ ] Build portfolio piece
- [ ] Other: ___________

---

## Step 6: Risk Summary

Compile all identified risks:

### High Priority Risks
| Risk | Impact | Mitigation |
|------|--------|------------|
| | High | |
| | High | |

### Medium Priority Risks
| Risk | Impact | Mitigation |
|------|--------|------------|
| | Medium | |
| | Medium | |

### Accepted Risks
| Risk | Why Accepted |
|------|--------------|
| | |

---

## Step 7: Prepare Handoff Document

Create the final synthesis document:

### ideation-handoff.md Structure:
```markdown
# Ideation Handoff: [Game Title]

## Executive Summary
[Vision statement from Step 4]

## Concept Overview
[Summary from concept-brief.md]

## Market Position
[Summary from market-analysis.md]

## Mechanics Overview
[Summary from mechanics-exploration.md]

## Success Criteria
[From Step 5]

## Known Risks
[From Step 6]

## Recommendations for Design Team
1. Priority focus areas
2. Potential challenges
3. Open questions to resolve

## Attachments
- Full concept-brief.md
- Full market-analysis.md
- Full mechanics-exploration.md
```

---

## Output

Create the handoff document at:
`_gdks-output/01-ideation/ideation-handoff.md`

This document will be the primary input for the Design Team.

---

## Next Steps

Once synthesis is complete:

1. **Review with stakeholders** (if applicable)
2. **Run `*validate`** to do final concept validation
3. **Run `*handoff`** to officially transition to Design Team
4. **Load `@game-design-director`** (Diana) to begin GDD creation

---

*Share your existing documents and let's synthesize them into a cohesive vision!* 🔄
