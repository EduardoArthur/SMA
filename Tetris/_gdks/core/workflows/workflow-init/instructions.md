# Workflow: Initialize GD-KS Project

## Purpose
This workflow helps you set up GD-KS workflow tracking for your game project and recommends the best development track based on your project's needs.

## When to Use
- Starting a new game project with GD-KS
- After running `npx gd-ks install`
- When you want to formalize your development approach

---

## Step 1: Gather Project Information

Let me learn about your project. Please answer the following questions:

### Basic Information
1. **Project Name**: What is your game called? (working title is fine)
2. **Game Type**: What genre/type of game? (e.g., platformer, RPG, puzzle)
3. **Brief Description**: In 1-2 sentences, what is your game about?

### Scope Assessment
4. **Team Size**: How many people will work on this?
   - Solo (1 person)
   - Small team (2-5 people)
   - Medium team (6-15 people)
   - Large team (15+ people)

5. **Timeline**: How much time do you have for development?
   - Prototype/Game Jam (1-4 weeks)
   - Short project (1-3 months)
   - Medium project (3-6 months)
   - Long project (6+ months)

6. **Experience Level**: Your experience with Unreal Engine?
   - Beginner (learning UE5)
   - Intermediate (some UE projects)
   - Advanced (experienced with UE)

7. **Documentation Needs**: How much documentation do you need?
   - Minimal (just enough to build)
   - Standard (good documentation for team)
   - Comprehensive (detailed docs for large team/investors)

---

## Step 2: Track Recommendation

Based on your answers, I'll recommend one of three tracks:

### 🚀 Quick Track
**Best for:** Solo developers, game jams, rapid prototyping
- Phases: Ideation → Engine
- Skip detailed design docs
- Focus on getting to playable quickly
- Estimated: 1-2 weeks of setup

### ⚖️ Standard Track
**Best for:** Small teams, short-medium projects
- Phases: Ideation → Design → Engine
- Good balance of documentation and speed
- Creates GDD and key design docs
- Estimated: 2-4 weeks of setup

### 📋 Full Track
**Best for:** Medium-large teams, long projects, investor pitches
- Phases: Ideation → Design → Planning → Engine
- Comprehensive documentation
- Sprint planning and project management
- Estimated: 4-8 weeks of setup

---

## Step 3: Initialize Workflow Status

After selecting your track, I'll create the `gdks-workflow-status.yaml` file that tracks your progress through the development process.

This file will be created at: `_gdks-output/gdks-workflow-status.yaml`

---

## Output

After completing this workflow, you'll have:

1. ✅ Workflow status file initialized
2. ✅ Recommended track selected
3. ✅ Clear next steps for your project
4. ✅ First agent recommendation

---

## Next Steps

After initialization, you'll typically want to:

1. **Quick Track**: Go directly to `@concept-brainstormer` with `*brainstorm`
2. **Standard Track**: Start with `@concept-brainstormer`, then move to Design Team
3. **Full Track**: Follow the complete pipeline from Ideation through Planning

---

*Type your answers to the questions in Step 1 to begin!*
