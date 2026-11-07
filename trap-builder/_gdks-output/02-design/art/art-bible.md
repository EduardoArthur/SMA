# Trap Builder - Art Bible (Mínima)

> **Autora:** Aurora 🎨 (Art Director) · **Versão:** 0.1 · **Data:** 2026-11-07
> **Escopo:** protótipo. Uma página de regras para produzir arte **vetorial simples**, sozinho, rápido e consistente.

---

## 1. Direção em Uma Frase

> **Cartoon clássico de perseguição (anos 40–50), achatado em vetor: formas redondas, contorno grosso marrom-escuro, cores chapadas e exagero no movimento, não no desenho.**

**Três regras de ouro:**
1. **Legibilidade antes de beleza:** cada armadilha precisa ser reconhecida pela **silhueta**, a 64 px.
2. **Movimento faz o cartoon:** desenhos simples + *squash & stretch* via **Tween** no Godot. Nada de animação quadro a quadro no protótipo.
3. **Dois mundos:** Blueprint (azul, linhas brancas, técnico) × Ação (colorido, quente, vivo). A transição entre eles é o "momento" do jogo.

---

## 2. Marca: Catapimba S.A. (G3 resolvido)

- Fornecedora fictícia de todas as engenhocas. Substitui qualquer referência a marcas reais.
- **Logo:** "CATAPIMBA S.A." em letras grossas arredondadas, levemente tortas, dentro de um carimbo oval, com uma estrelinha de impacto (💥) no lugar do pingo do "i".
- **Onde aparece:** caixotes de TNT, lateral da bigorna, base do ventilador, título do catálogo ("Catálogo Catapimba, Primavera"), tela de resultado ("Entregue por Catapimba S.A.").
- **Tom:** o catálogo fala como propaganda dos anos 50 ("NOVIDADE! Bigorna Modelo 3 — agora com 20% mais queda!").

---

## 3. Paletas

### 3.1 Modo Blueprint
| Uso | Cor |
|-----|-----|
| Fundo | `#1F4E8C` |
| Grade (linhas finas) | `#3A6FB0` |
| Traço principal (rotas, contornos) | `#F2F6FF` |
| Rotas iluminadas | `#FFD84D` |
| Tile válido (destaque ao posicionar) | `#7FE0FF` (40% de opacidade) |
| Barranco (perigo) | Hachura diagonal `#FF6B5A` |
| Aresta bloqueada (prévia) | `#F2F6FF` tracejado |

### 3.2 Modo Ação
| Uso | Cor |
|-----|-----|
| Grama | `#7BC950` · sombra `#4E9A3A` |
| Trilha (rota) | `#D9A066` · borda `#B07C45` |
| Árvores e arbustos (bloqueado) | `#2F6B3A` · `#3F8A4A` |
| Barranco | Borda `#6B4A2E` · fundo `#2B1D14` |
| Tile marcado | Toco de árvore/pedra plana `#9C8A6E` |
| **Contorno universal** | `#2B1B17` (marrom quase preto; nunca preto puro) |

### 3.3 Personagens e armadilhas
| Elemento | Cores |
|----------|-------|
| Porco | Corpo `#F4A6B8`, focinho `#E77F9B`, bochechas `#FF8FA8` |
| Lobo | Pelo `#7A7F8C`, barriga/focinho `#C9CDD6`, olhos `#FFE45C` |
| Bigorna | `#4A4E57`, brilho `#8A909C` |
| TNT | Caixote `#D63A2F`, faixa `#F2E3C2`, pavio `#E8B33A` |
| Isca (tigela) | Tigela `#3D7DD9`, lavagem `#A3C244` |
| Ventilador | Grade `#9AA3AD`, base `#5C6470`, vento `#FFFFFF` (60%) |
| Piche | `#1E1A24`, brilho `#4A4458` |

### 3.4 UI (estilo catálogo de papel)
| Uso | Cor |
|-----|-----|
| Papel | `#F2E3C2` |
| Tinta (texto) | `#2B1B17` |
| Moeda / destaque | `#E8B33A` |
| Perigo / fuga | `#D63A2F` |
| Estrela ganha / vazia | `#FFD84D` / `#8A7A5C` |

---

## 4. Especificações Técnicas (Godot 4)

| Item | Regra |
|------|-------|
| Tile | **64×64 px** no jogo; desenhar a **128×128** e exportar PNG (2× para telas densas/mobile) |
| Contorno | 6 px a 128 (= 3 px no jogo), cantos arredondados |
| Personagens | Cabem em 1 tile (pé no centro do tile); até 96×96 a 128 se precisar "sair" do tile |
| Câmera | **Top-down ortográfica**. Personagens e objetos em **3/4 frontal** (vistos levemente de frente) |
| Direção dos personagens | **Um único sprite de perfil**, espelhado (`flip_h`) para esquerda/direita; em movimento vertical, mantém o último lado |
| Formato | PNG com alfa; filtro **Linear** (vetor), não Nearest |
| Ferramentas | Inkscape (grátis) ou Figma → export PNG 2× |
| Organização | `res://art/{tiles,pigs,wolf,traps,ui,fx}/nome_variante.png` em snake_case |
| TileSet | 1 TileSet para o Ação + 1 para o Blueprint, **mesmo layout de células**, com a Custom Data `tile_type` |

---

## 5. Personagens

### Porco (inimigo)
- **Forma:** feijão rosa com 4 perninhas curtas e orelhas triangulares; olhos grandes e espertos (presa esperta, não vítima boba).
- **Assets:** `pig_idle`, `pig_walk_a`, `pig_walk_b` (2 quadros alternados), `pig_flat` (achatado pela bigorna), `pig_eat`, `pig_dizzy` (capacete, pós-protótipo).
- **Tweens:** andar = balanço vertical de ±3 px a cada passo; piche = esticar e encolher 10% mais devagar; empurrão = rotação de 15° + deslize; captura = *squash* (escala 1,3 × 0,7) e sumir com estrelinhas.

### Lobo (protagonista, D007)
- **Forma:** alto, magro, focinho comprido, orelhas pontudas, sorriso de canto. Chapéu-coco e gravata-borboleta (vilão cavalheiro).
- **Pose fixa:** atrás de um arbusto num canto do mapa, com binóculo.
- **Assets (6 expressões, um PNG cada):** `wolf_watch` (binóculo), `wolf_lick` (lambe os beiços), `wolf_cheer` (guardanapo e talheres), `wolf_angry` (arranca o chapéu), `wolf_feast` (★★★), `wolf_sad` (derrota).
- **Tween de reação:** pulo de 8 px + *squash* ao trocar de expressão.

---

## 6. Armadilhas (silhuetas distintas)

| Armadilha | Silhueta | Estados / VFX |
|-----------|----------|---------------|
| Bigorna | Bloco trapezoidal pesado com chifre de lado | Sombra elíptica crescendo → queda (tween rápido) → poeira → fica no tile |
| TNT | Caixote quadrado com "TNT" + Catapimba S.A. | Pavio soltando faísca → flash branco → nuvem de fumaça em 3 círculos |
| Isca | Tigela baixa e larga com lavagem verde | Linhas de cheiro onduladas subindo (indica que está ativa) → vazia |
| Ventilador | Círculo com grade sobre base quadrada; **seta grande indica a direção** | Linhas de vento animadas na corrente (3 tiles) |
| Piche | Poça irregular escura com brilho | Bolhas lentas (tween de escala) |

**No Blueprint:** cada armadilha tem uma versão em **traço branco** (só contorno, sem preenchimento), como um desenho técnico.

---

## 7. Transição Blueprint → Ação (o "momento")

1. **0–300 ms:** as linhas brancas do Blueprint se apagam da esquerda para a direita.
2. **200–700 ms:** a camada colorida aparece por trás com um wipe diagonal.
3. **700 ms:** o lobo aparece no arbusto com um "pop".

**Implementação simples:** duas `TileMapLayer` sobrepostas + um `ShaderMaterial` com uniform `progress` (0→1), animado por um Tween, que faz `discard` de pixels com `UV.x + UV.y*0.3 < progress`. Alternativa ainda mais simples: fade de `modulate.a`.

---

## 8. UI

- **Catálogo:** painel de papel `#F2E3C2` com bordas recortadas; cada item é um "anúncio" com ícone, nome e preço com moeda.
- **Fontes (gratuitas, Google Fonts / OFL):** títulos **Luckiest Guy** · texto **Nunito Bold**.
- **HUD:** moedas no canto superior esquerdo, fugas permitidas (ícones de porco) no superior direito, botão **JOGAR** grande (botão vermelho de máquina).
- **Resultado:** 3 estrelas grandes que "carimbam" uma a uma (tween de escala 1,5 → 1,0).

---

## 9. Lista de Assets do Protótipo (≈ 40 PNGs)

| Categoria | Assets |
|-----------|--------|
| Tiles Ação | grama ×2, trilha (reta, curva, T, cruz), árvore ×2, arbusto, barranco (borda + fundo), tile marcado |
| Tiles Blueprint | as mesmas formas em traço branco, + hachura do barranco |
| Porco | idle, walk_a, walk_b, flat, eat |
| Lobo | 6 expressões + arbusto |
| Armadilhas | 5 × (Ação + Blueprint) + bigorna caída + sombra |
| FX | poeira, fumaça, estrelinhas, linhas de vento, linhas de cheiro, bolha |
| UI | painel de papel, botão JOGAR, moeda, ícone de porco, estrela cheia e vazia, logo Catapimba |

---

*Art Bible por Aurora, GD-KS Art Director*
