# VKWIND11 → 30 FPS: Risk of Rain 2 на Helio G99 Ultra

## Текущая ситуация

| Параметр | Значение |
|----------|----------|
| Игра | Risk of Rain 2 (Unity, D3D11) |
| Базлайн | Proton 11.0 + DXVK 1.10.3 = **5–15 FPS** |
| Цель | VKWIND11 = **30 стабильных FPS** |
| Железо | Poco M6 Pro, Helio G99 Ultra / Mali-G57 MC2 |
| GPU | 128 GFLOPS, 32 execution units, Vulkan 1.1 |
| RAM | 6 GB |
| Стек | Winlator → Wine/Box64 (x86→ARM64) → VKWIND11 → Vulkan → Mali |

## Анализ узких мест (где теряются FPS)

| Узкое место | Вклад | Решение |
|---|---|---|
| Box64 CPU overhead (x86→ARM64) | ~40-50% | Напрямую не решаем, но минимизируем вызовы |
| DXVK shader compilation stalls | ~20% | Асинхронная компиляция в VKWIND11 |
| DXVK memory overhead | ~15% | Наш более лёгкий memory manager |
| Mali fillrate / overdraw | ~15% | Оптимизация шейдеров, reduce overdraw |

## Архитектурный план (4 фазы)

### Фаза 1 — Профилирование (день 1-2)

**Цель:** Определить CPU-bound (Box64) или GPU-bound (Mali fillrate).

- [ ] Включить `VKWIND_LOG_LEVEL=4` (Debug), собрать лог запуска RoR2
- [ ] Замерить: CPU% (Box64 vs GPU), время кадра breakdown
- [ ] Определить основной bottleneck: Box64 JIT или Mali shader throughput
- [ ] Инструмент: Mali Offline Compiler + `VK_KHR_performance_query`
- [ ] Записать baseline: avg FPS, 1% low, frame time p50/p95/p99

### Фаза 2 — Async Pipeline + Descriptor Pool (день 3-7)

**Главный выигрыш.** Сейчас каждый кадр RoR2 может пересоздавать pipeline.

```
Текущее: каждый Draw → pipeline lookup → miss → sync compile → hitch
Цель:    первый Draw → async compile → 2-3 кадра fallback → готово
```

- [ ] **Async pipeline compilation**: worker thread компилирует PSO пока рендерится fallback
  - В `create_or_get_pipeline()`: при miss → queue job → return fallback
  - Worker → compile real PSO → store in `m_pipelineCacheMap` → swap on next frame
- [ ] **Descriptor pool**: одинаковые layout → один pool, переиспользование
  - Текущее: 8 текстур = 8 × `vkUpdateDescriptorSets`
  - Цель: 1 пул, 1 × `vkUpdateDescriptorSets` на кадр
- [ ] **Pipeline cache**: `m_pipelineCacheMap` — правильное ключевание по PSO desc hash
- [ ] Замерить: shader compile time до/после (цель <5ms async)

### Фаза 3 — Mali-специфичные оптимизации (день 8-14)

- [ ] **Push constants вместо UBO**: Mali-G57 лучше с push constants для частых обновлений (MVP уже через push constants)
- [ ] **Texture format optimization**: Mali любит `VK_FORMAT_R8G8B8A8_UNORM` (native), избегать `B8G8R8A8` swizzle
- [ ] **Early-Z**: Mali поддерживает — убедиться что depth write на правильном stage
- [ ] **Reduce overdraw**: RoR2 много частиц/эффектов — alpha test discard на раннем этапе
- [ ] **Sampler caching**: `m_samplerCache` — LRU-ограничение
- [ ] **State change batching**: объединить Draw-вызовы с одинаковым состоянием

### Фаза 4 — Measurement & Tuning (день 15-21)

- [ ] Замерить на том же запуске RoR2
- [ ] Целевые метрики:

```
Метрика:                      Цель:
├── Average FPS              30+ стабильных
├── 1% low FPS               25+ (без провалов)
├── Frame time               <33ms (p95)
├── Shader compile time       <5ms (async)
└── Memory usage              <500MB (vs DXVK ~800MB)
```

- [ ] Сравнить с baseline: DXVK 1.10.3 (5-15 FPS)
- [ ] Итерации: если <30 FPS → вернуться к Фазе 3 с данными профилирования

## Ключевые технические решения

### 1. Async Pipeline (самый большой выигрыш)

```cpp
// В create_or_get_pipeline():
// При miss → queue job в worker thread → return fallback
// Worker → compile real PSO → store in cache → swap on next frame
// Результат: первый кадр с fallback, дальше — полная скорость
```

### 2. Descriptor Set batching

```cpp
// Текущее: 8 текстур = 8 vkUpdateDescriptorSets
// Цель: 1 пул, 1 vkUpdateDescriptorSets на кадр
// Mali: descriptor update = driver overhead, batch = win
```

### 3. Shader translation caching

```
RoR2 использует ~20-30 уникальных шейдеров
Текущее: каждый = полный SM4→SPIR-V (может быть 2-5ms)
Цель: cache by bytecode hash, инвалидация только при смене
```

### 4. Mali GPU frequency awareness

```
Mali-G57 MC2: 4-core, 1.1 GHz max
DVR: dynamic voltage scaling
→ Не hammer GPU с лишними draw calls
→ Batch state changes
```

## Риски

| Риск | Вероятность | Митигация |
|---|---|---|
| Box64 CPU bottleneck — не решается VKWIND | Высокая | Профилирование покажет; если CPU-bound → нужен оптимизированный Box64 |
| Mali driver bugs | Средняя | Fallback paths, validation layers |
| RoR2 specific quirks (Unity D3D11) | Средняя | Game-specific workarounds |
| Descriptor pool exhaustion | Низкая | Pool sizing, reset between frames |

## Оценка времени

| Фаза | Дни | Ожидаемый FPS |
|---|---|---|
| Профилирование | 2 | 5-15 (baseline confirmed) |
| Async pipeline + descriptors | 5 | 15-22 |
| Mali optimizations | 7 | 22-30 |
| Measurement + tuning | 7 | **30 стабильных** |
| **Итого** | **21 день** | |

## Если Box64 — bottleneck (план B)

Если профилирование покажет что 70%+ времени в Box64 (CPU translation):

- VKWIND11 оптимизации дадут только +5-10 FPS
- Нужно оптимизировать **Box64** (JIT cache, thread pool)
- Или использовать **FEX-Emu** вместо Box64 для x86→ARM64
- Или комбинация: VKWIND11 + оптимизированный Box64

**Начинаем с профилирования — без данных любые оптимизации это гадание.**
