#!/usr/bin/Rscript

suppressPackageStartupMessages(library(gdata))
#suppressPackageStartupMessages(library(gplots))
suppressPackageStartupMessages(library(ggplot2))
suppressPackageStartupMessages(library(reshape2))

args <- commandArgs(T)

if (length(args) != 1)
    stop("Usage: plot_vars.R <some_output_of_tiny_var.dat>")
file <- args[1]

src <- read.csv(args[1], F, " ")
src <- as.data.frame(t(src))
lsrc <- (log2(src + 1) - 14.427)*3.0/5.0 + 0.5

# CDF plot
probs <- seq(0, 1, 1/7)
cdf <- sapply(lsrc, quantile, probs = probs, na.rm = T)
cdf <- melt(cdf, varnames=c("Quantile", "Frame"), value.name="Variance")
cdf$Quantile <- ordered(reorder.factor(cdf$Quantile))
cdf$Frame <- ordered(reorder.factor(cdf$Frame))

cdf_plot <- ggplot(cdf, aes(Frame, Variance, colour=Quantile, group=Quantile)) + geom_path() + scale_y_continuous(breaks = seq(-20, 20, 0.5)) # + coord_cartesian(xlim=c(0,20))
cdf_plot_name <- paste(file, "cdf_plot.png", sep=".")
png(cdf_plot_name, 1920, 1050)
print(cdf_plot)
dev.off()

# Frequency plot
var <- as.data.frame(sapply(lsrc, as.integer))
freq <- melt(var, variable.name="Frame", value.name="Variance")

freq_plot <- qplot(Frame, data=freq, geom="bar", fill=factor(Variance)) + geom_bar(width=1)
freq_plot_name <- paste(file, "freq_plot.png", sep=".")
png(freq_plot_name, 1920, 1050)
print(freq_plot)
dev.off()
