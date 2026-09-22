# Customer Intelligence — Protect, Grow, Expand

## Executive summary

This project turns real-world commerce transaction data into three commercial decisions:

**Protect** existing revenue, **Grow** incremental product revenue, and **Expand** customer value.

The core dataset is the public Instacart Market Basket Analysis dataset. It is used as a proxy for a DTC / e-commerce transaction environment; it is **not WEEM production data**. The dataset gives us longitudinal customer behavior, product relationships, and observed future behavior for validation.

The project is built around a decision loop rather than descriptive reporting:

**Business problem → signal → validation → target population → action → experiment → KPI → economics → scale / iterate / stop**

## Dataset

- **206,209 customers**
- **3,421,083 orders**
- **33,819,106 PRIOR + TRAIN order-product records**
- **49,688 products**
- **21 departments**
- **131,209 TRAIN orders** reserved for observed future outcomes

We chose the dataset for analytical depth and decision value, not simply for maximum row count.

## The three commercial workstreams

### PROTECT — Protect existing revenue

**Finding:** Customers with persistent behavioral deterioration relative to their own baseline remained slower in later purchasing windows.

**Target population:** 6,763 customers.

**Final signal:** +1.87 days vs. baseline in orders 26–30 and +1.91 days vs. baseline in orders 31–35.

**Action:** Prioritize this population for a controlled retention intervention.

**Primary KPI:** Incremental retention / subsequent-order activity.

### GROW — Create incremental revenue

**Finding:** The Black Beans → Chicken Breasts relationship showed higher future adoption in the exposed population than baseline in the held-out validation data.

**Validation population:** 847 exposed customers; 24 observed adopters.

**Final signal:** 0.463% baseline adoption vs. 2.834% exposed adoption, a +2.37 percentage-point gap.

**Action:** Test a targeted recommendation against business-as-usual control.

**Primary KPI:** Incremental target-product adoption.

### EXPAND — Increase customer value

**Finding:** Established customers with relatively narrow portfolio breadth expanded into new departments more often than the comparison group.

**Target definition:** 20+ historical orders and 10 or fewer departments at the order-20 baseline.

**Validation population:** 4,758 customers.

**Final signal:** 64.84% expansion vs. 53.71% for non-target customers, a +11.13 percentage-point gap.

**Priority categories:** Frozen (+8.63 pp), Pantry (+7.66 pp), Bakery (+6.79 pp), Deli (+6.68 pp).

**Action:** Test a focused category-expansion experience against business-as-usual control.

## Measurement framework

### Behavioral lift
- PROTECT: incremental retention / subsequent-order activity
- GROW: incremental target-product adoption
- EXPAND: incremental department adoption

### Commercial impact
- Incremental revenue per customer

### Economic value
- Incremental contribution margin per customer

### Efficiency
- Contribution margin per campaign dollar
- Campaign cost per incremental adopter

### Decision rule
- **Scale:** positive incremental lift, acceptable economics, clean guardrails
- **Iterate:** promising but weak, heterogeneous, or operationally improvable result
- **Stop:** no incremental value or unfavorable economics / guardrails

## Repository structure

```text
sql/
├── 01_staging/
├── 02_conformed/
├── 03_marts/
├── 04_exploration/
└── 04_ml_features/

data/gold/             # small validation outputs only
results/               # final result tables used by the executive story
python/validation/     # reproducibility / validation helpers
docs/                  # methodology, data dictionary, limitations, decision log
dashboard/             # lightweight executive HTML preview
presentation/          # final executive presentation
```

## SQL

The SQL is organized by analytical layer rather than pasted into one file. Core transformations are versioned separately so a reviewer can trace the path from raw transaction data to reusable marts.

The SQL uses BigQuery-style syntax and `project.dataset` placeholders. Replace those identifiers with the target environment before execution.

## Reproducibility and limitations

The repository does **not** include the full raw Instacart files or local database files. Raw data remains external to the public repository.

The dataset does not contain WEEM-specific prices, revenue, margin, discounts, inventory, supplier, fulfillment, campaign exposure, or CRM treatment fields. Historical signals therefore identify **testable opportunities**, not causal revenue forecasts.

Business economics should be connected only after a live experiment produces causal incremental lift and actual company economics are available.
