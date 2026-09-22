# Methodology

## 1. Business framing

The project starts from three commercial questions:

1. Which customers show persistent behavioral deterioration that could justify a retention intervention?
2. Which product relationships predict future adoption above baseline strongly enough to justify a cross-sell test?
3. Which highly engaged customers with relatively narrow portfolios are most suitable for category-expansion testing?

## 2. Analytical principles

### Customer-relative baselines

Where possible, behavior is compared with the customer's own earlier behavior rather than using one global threshold.

### Temporal integrity

Features and validation windows are ordered by customer order sequence. Future outcomes are kept separate from the behavioral baseline used to define the target population.

### Baseline comparison

For product adoption, the key question is not simply whether two products co-occur. The analysis compares adoption among exposed customers with a baseline population.

### Holdout / out-of-time validation

Candidate patterns are checked in later orders rather than relying only on the same period used to discover the signal.

### Business decision layer

Every validated signal is translated into a target population, commercial action, KPI, business target, and scale/iterate/stop rule.

## 3. W1 logic

Persistent deterioration is defined using customer-relative cadence behavior across successive windows. A single bad period is not sufficient for the final intervention candidate.

The final W1 candidate population is the persistent-deterioration segment of 6,763 customers.

## 4. W2 logic

Product pairs are used for candidate generation, but candidate selection is not based on co-purchase alone. The final evidence layer compares future adoption for customers exposed to product A with a baseline population for product B and then checks holdout evidence volume and direction.

The dashboard uses the Black Beans → Chicken Breasts relationship as the validated example.

## 5. W3 logic

The baseline portfolio is measured by department breadth at order 20. The target segment is customers with 20+ orders and no more than 10 departments at order 20. Expansion is measured as adoption of a department not present in the baseline portfolio during the validation window.

## 6. Commercial validation

Historical gaps are opportunity signals, not causal forecasts. Production rollout requires randomized treatment/control measurement.
