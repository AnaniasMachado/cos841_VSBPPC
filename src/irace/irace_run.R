library(irace)

parameters <- readParameters("parameters.txt")
scenario <- defaultScenario()
scenario$instances <- read.table("sampled_instances.txt", stringsAsFactors = FALSE)[[1]]
scenario$parameters <- parameters

scenario$maxExperiments <- 1000
scenario$parallel <- 1
scenario$debugLevel <- 1
scenario$logFile <- "irace_logFile.Rdata"
scenario$recoveryFile <- "irace_recoveryFile.Rdata"
scenario$softRestart <- TRUE
scenario$firstTest <- 2
scenario$eachTest <- 2
scenario$deterministic <- 0
scenario$targetRunner <- "target.R"

# Run irace (it will resume automatically if log exists)
cat("Starting or resuming irace run...\n")
results <- irace(scenario = scenario)

# Save final results
save(results, file = "irace_results.Rdata")
write.csv(results, "irace_ranked.csv", row.names = FALSE)
cat("irace tuning completed.\n")