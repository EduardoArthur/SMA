/**
 * GD-KS Template Conditionals
 *
 * Minimal mustache-ish renderer for engine-conditional blocks in templates:
 *
 *   {{#engine unreal-5}}
 *   Use UPROPERTY macros to expose fields to Blueprint.
 *   {{/engine}}
 *
 *   {{#engine godot-4}}
 *   Use @export annotations in GDScript.
 *   {{/engine}}
 *
 *   {{#engine-not unreal-5}}
 *   (shown for any engine OTHER than unreal-5)
 *   {{/engine-not}}
 *
 * Plus simple variable substitution: {{engine.id}}, {{engine.name}}.
 *
 * Introduced in v0.4 Sprint 3.
 *
 * Kept deliberately tiny — no includes, no loops, no conditionals beyond
 * engine matching. More complex templating is out of scope.
 */

/**
 * Render a template string against an engine context.
 *
 * @param {string} template - the raw template text
 * @param {object} context
 * @param {string} context.engineId - e.g. 'unreal-5'
 * @param {object} [context.engine] - engine-profile.yaml contents
 * @returns {string}
 */
export function renderTemplate(template, context) {
  if (typeof template !== 'string') {
    throw new TypeError('template must be a string');
  }
  if (!context || !context.engineId) {
    throw new TypeError('context.engineId is required');
  }

  let out = template;

  // 1. Positive engine blocks: {{#engine ID}}...{{/engine}}
  out = out.replace(
    /\{\{#engine\s+([a-z][a-z0-9-]*)\}\}([\s\S]*?)\{\{\/engine\}\}/g,
    (_, engineId, content) => (engineId === context.engineId ? content : '')
  );

  // 2. Negative engine blocks: {{#engine-not ID}}...{{/engine-not}}
  out = out.replace(
    /\{\{#engine-not\s+([a-z][a-z0-9-]*)\}\}([\s\S]*?)\{\{\/engine-not\}\}/g,
    (_, engineId, content) => (engineId === context.engineId ? '' : content)
  );

  // 3. Simple variable substitution
  out = out.replace(/\{\{\s*engine\.id\s*\}\}/g, context.engineId);
  if (context.engine?.engine?.name) {
    out = out.replace(/\{\{\s*engine\.name\s*\}\}/g, context.engine.engine.name);
  }

  // 4. Collapse the blank lines left by removed blocks (3+ newlines → 2)
  out = out.replace(/\n{3,}/g, '\n\n');

  return out;
}

/**
 * Render a template file on disk against an engine context.
 */
export async function renderTemplateFile(path, context) {
  const { readFile } = await import('fs/promises');
  const raw = await readFile(path, 'utf8');
  return renderTemplate(raw, context);
}
