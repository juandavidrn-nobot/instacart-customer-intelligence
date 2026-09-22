# Instacart Assessment — Development Log

## Purpose

This document is the living record of the project: what we decided, why we decided it, what changed, what evidence caused the change, and what remains uncertain.

## Entry 001 — Project direction
**Status:** Locked

**Decision:** Use the public Instacart Market Basket Analysis dataset as the core raw material.

**Reason:** It provides a real, multi-table, longitudinal customer purchase history with ~3.4M orders, ~206K customers, ~49.7K products, and ~33.8M order-product observations. Its temporal structure supports rigorous out-of-time validation and behavioral analysis.

**Important constraint:** It does not contain inventory, supplier, fulfillment, delivery, margin, or subscription data. We will not invent conclusions about those areas.

## Entry 002 — Business focus
**Status:** Locked at workstream level; findings not predetermined.

The project will investigate four business workstreams:

1. Customer Intelligence & Lifecycle
2. Product & Basket Intelligence
3. Demand Intelligence for Planning
4. Customer Similarity & Incremental Growth

Predictive modeling is a cross-cutting validation capability, not the business objective by itself.

## Entry 003 — Assessment philosophy
**Decision:** Optimize for decision quality, analytical rigor, and traceable reasoning—not raw dataset size or model accuracy.

Every major conclusion must have:
- a clearly stated question;
- a defensible measurement;
- an alternative explanation considered;
- validation or robustness testing;
- a business implication;
- an explicit limitation where applicable.

## Entry 004 — Analytical architecture
**Status:** In progress

We will build a layered architecture:

RAW → STAGING → CONFORMED → ANALYTICAL MARTS → ML FEATURES → INSIGHTS → DECISION LAYER

Core grains:
- order
- order-item
- customer
- product
- customer × product
- basket/order

## Entry 005 — Temporal integrity
**Decision:** Any predictive feature must obey an as-of cutoff. Future order information cannot be used to construct features for a prediction point.

The original `train` orders will be treated as a final benchmark only after internal historical backtesting is developed.

## Entry 006 — Known data limitation
**Decision:** `days_since_prior_order` must be interpreted carefully because its observed maximum is 30; values at 30 represent a censored/capped interval rather than necessarily an exact 30-day interval.

## Entry 007 — Current phase
**Current phase:** Phase 3 — Analytical Architecture

**Next checkpoint:** finalize table-grain specification, conformed model, feature definitions, temporal policy, and reproducible SQL structure before advanced analytics.
