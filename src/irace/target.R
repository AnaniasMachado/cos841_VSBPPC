#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)

# -------------------- Check enough arguments --------------------
if (length(args) < 4) {
  stop("Not enough arguments: irace should pass at least 4 arguments ",
       "(run_id, seed, experiment, instance_file)")
}

# -------------------- Positional arguments --------------------
# irace system arguments: run_id, seed, experiment, instance
run_id <- args[1]
seed <- args[2]
experiment <- args[3]
instance_arg <- args[4]

# Remaining arguments are irace parameter flags
optional_args <- if (length(args) > 4) args[5:length(args)] else character(0)

# -------------------- Helper to get optional flags --------------------
get_arg <- function(flag, default = NULL) {
  idx <- which(optional_args == flag)

  if (length(idx) > 0 && idx[1] < length(optional_args)) {
    return(optional_args[idx[1] + 1])
  }

  return(default)
}

# -------------------- Parse irace instance line --------------------
# Expected format:
#   instance_path,bin_option,cost_option
instance_parts <- strsplit(instance_arg, ",", fixed = TRUE)[[1]]

if (length(instance_parts) != 3) {
  stop("Invalid instance format. Expected: instance_path,bin_option,cost_option. Got: ",
       instance_arg)
}

instance_file <- instance_parts[1]
bin_size <- instance_parts[2]
cost_type <- instance_parts[3]

# -------------------- Infer instance set --------------------
if (grepl("set_1", instance_file, fixed = TRUE)) {
  set_type <- "SET1"
} else if (grepl("set_2", instance_file, fixed = TRUE)) {
  set_type <- "SET2"
} else {
  stop("Could not infer set_type from instance path: ", instance_file)
}

# -------------------- Tunable parameters --------------------
kw <- get_arg("--kw", "400")
kc <- get_arg("--kc", "275")

max_it <- get_arg("--max_it", "1000")
max_no_imp <- get_arg("--max_no_imp", "25")

acceptance <- get_arg("--acceptance", "BEST")
builder <- get_arg("--builder", "IBFD")
exact_cover <- get_arg("--exact_cover", "0")

# -------------------- Fixed runner parameters --------------------
# These match the defaults in ails_target.cpp unless overridden here.
time_limit <- get_arg("--time_limit", "600")
verbose <- get_arg("--verbose", "0")

# -------------------- Resolve instance path --------------------
# Keep the relative path from the sampled instances file.
# Do not normalizePath(), because the absolute project path contains spaces.
if (!file.exists(instance_file)) {
  stop("Error: instance file not found from working directory ",
       getwd(), ": ", instance_file)
}

# -------------------- Build command for VSBPPC AILS target --------------------
ails_binary <- "../bin/ails_target"

if (!file.exists(ails_binary)) {
  stop("Error: ails_target binary not found from working directory ",
       getwd(), ": ", ails_binary)
}

cmd_args <- c(
  instance_file,
  "--set_type", set_type,
  "--cost_type", cost_type,
  "--bin_size", bin_size,
  "--kw", kw,
  "--kc", kc,
  "--max_it", max_it,
  "--max_no_imp", max_no_imp,
  "--acceptance", acceptance,
  "--builder", builder,
  "--exact_cover", exact_cover,
  "--time_limit", time_limit,
  "--verbose", verbose,
  "--seed", seed
)

# -------------------- Run AILS --------------------
# Use explicit shell quoting so paths with spaces are safe if they appear.
cmd <- paste(
  shQuote(ails_binary),
  paste(shQuote(cmd_args), collapse = " ")
)

res <- tryCatch(
  system(cmd, intern = TRUE, ignore.stderr = FALSE),
  error = function(e) stop("Failed to run ails_target: ", e$message)
)

# -------------------- Parse numeric result strictly --------------------
numeric_res <- suppressWarnings(as.numeric(res))
numeric_res <- numeric_res[!is.na(numeric_res)]

if (length(numeric_res) != 1) {
  stop("Error: ails_target returned non-numeric or multiple outputs:\n",
       paste(res, collapse = "\n"))
}

score <- numeric_res[1]

# -------------------- Output strictly for irace --------------------
cat(score, "\n")