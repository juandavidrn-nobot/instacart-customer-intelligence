# SQL layers

The repository does not put every exploratory query into one large file. The final, reusable SQL is split by purpose:

- `01_staging/` — canonical source tables
- `02_conformed/` — conformed facts and dimensions
- `03_marts/` — reusable analytical marts
- `04_exploration/` — documented exploratory / validation walkthroughs
- `04_ml_features/` — reserved for prediction-time feature definitions

Exploratory queries that were abandoned or superseded are not treated as final production logic. The executive numbers in `results/` and the methodology in `docs/` are the source of truth for the final story.
