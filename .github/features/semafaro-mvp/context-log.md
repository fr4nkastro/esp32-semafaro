# SEMAFARO MVP - Context Engineering Log

## Context Strategy Decisions
**Timestamp**: 2025-09-21 Initial Implementation

### Selected Context Strategy: WRITE + SELECT + COMPRESS
- **WRITE CONTEXT**: Maintaining implementation logs in `.github/semafaro-mvp/`
- **SELECT CONTEXT**: Using semantic_search for ESP32/Arduino patterns, read_file for existing docs
- **COMPRESS CONTEXT**: Hierarchical documentation structure, focused task breakdown
- **ISOLATE CONTEXT**: Single-agent approach for MVP scope

### Memory Patterns Identified
- **Hybrid Architecture**: POO device encapsulation + FSM control logic
- **Non-blocking Execution**: Scheduler::tick() pattern, update(nowMs) methods
- **Pin Management**: Centralized in core/Pins.hpp with constexpr
- **Safety Patterns**: Debounce 30ms, timeout handling, fault states

### Context Sources
- Primary: `/docs/lineamientos.md`, `/docs/PRD-MVP.md`
- Secondary: ESP32-S3 documentation via context7 if needed
- Hardware: ESP32Servo library docs, GPIO patterns

### Optimization Notes
- Focus on established patterns from lineamientos.md
- Leverage existing project structure in `/src`
- Use context7 only for ESP32-specific APIs if documentation gaps found