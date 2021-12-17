library(tidyr)
library(ggplot2)
library(ggthemes)
library(tibble)
library(dplyr)
library(patchwork)
library(scales)

theme_set(theme_clean())

threads_data <- read.csv("./threads_ryzen.csv", header = TRUE)

# data <- gather(threads, ) %>%
# data <- gather(data, variable, value, sddm_time, diag_max_time, create_time, b_min_reduce_time) %>%
# data <- pivot_longer(data, c(sddm_time, diag_max_time, create_time, b_min_reduce_time, b_min_critical_time, b_min_tree_time), names_to = "variable", values_to = "value") %>%

threads <- gather(threads_data, variable, value, sddm_time, diag_max_time, create_time, b_min_reduce_time) %>% select(variable, value, threads)
threads_crit_tree <- gather(threads_data, variable, value, b_min_critical_time, b_min_tree_time) %>% select(variable, value, threads)
# threads_tree <- gather(threads_data, variable, value, b_min_tree_time) %>% select(variable, value, threads)

color_labs <- c(
    "sddm_time" = "Check if A is strictly diagonally dominant",
    "diag_max_time" = "Find A's diagonal's max",
    "create_time" = "Create B",
    "b_min_reduce_time" = "Find B's min with reduction",
    "b_min_critical_time" = "Find B's min with critical section",
    "b_min_tree_time" = "Find B's min with tree"
)

breaks_order <- c(
    "sddm_time",
    "diag_max_time",
    "create_time",
    "b_min_reduce_time",
    "b_min_critical_time",
    "b_min_tree_time"
)

my_theme <- theme(
    # legend.position = c(.05, .85),
    legend.position = "right",
    legend.key.size = unit(.5, "cm"),
    legend.text = element_text(size = 7)
)

graph_threads <- ggplot(data = threads, aes(x = threads, y = value, color = variable)) +
    geom_path() +
    geom_point() +
    scale_x_continuous(trans = "log2", breaks = c(1, 2, 4, 8, 16, 32)) +
    scale_y_continuous(labels = scientific) +
    scale_color_discrete(labels = color_labs, breaks = breaks_order) +
    labs(y = "Time (s)", x = "# of Threads", color = "Operation") +
    my_theme

graph_threads_crit_tree <- ggplot(data = threads_crit_tree, aes(x = threads, y = value, color = variable)) +
    geom_path() +
    geom_point() +
    scale_x_continuous(trans = "log2", breaks = c(1, 2, 4, 8, 16, 32)) +
    scale_y_continuous(trans = "log", labels = scientific) +
    scale_color_discrete(labels = color_labs, breaks = breaks_order) +
    labs(y = "Time log(s)", x = "# of Threads", color = "Operation") +
    my_theme

# graph_threads_tree <- ggplot(data = threads_tree, aes(x = threads, y = value, color = variable)) +
#     geom_path() +
#     geom_point() +
#     scale_x_continuous(trans = "log2", breaks = c(1, 2, 4, 8, 16, 32)) +
#     scale_y_continuous(labels = scientific) +
#     scale_color_discrete(labels = color_labs, breaks = breaks_order) +
#     labs(y = "Time (s)", x = "# of Threads", color = "Operation") +
#     my_theme

sizes_data <- read.csv("./sizes_ryzen.csv", header = TRUE)

# data <- gather(threads, ) %>%

sizes <- gather(sizes_data, variable, value, sddm_time, diag_max_time, create_time, b_min_reduce_time) %>% select(variable, value, size)
sizes_crit_tree <- gather(sizes_data, variable, value, b_min_critical_time, b_min_tree_time) %>% select(variable, value, size)
# sizes_tree <- gather(sizes_data, variable, value, b_min_tree_time) %>% select(variable, value, size)

graph_sizes <- ggplot(data = sizes, aes(x = size, y = value, color = variable)) +
    geom_path() +
    geom_point() +
    scale_x_continuous(trans = "log2", breaks = c(8, 16, 32, 64, 128, 256, 512, 1024)) +
    scale_y_continuous(labels = scientific) +
    scale_color_discrete(labels = color_labs, breaks = breaks_order) +
    labs(y = "Time (s)", x = "Size of Matrix", color = "Operation") +
    my_theme

graph_sizes_crit_tree <- ggplot(data = sizes_crit_tree, aes(x = size, y = value, color = variable)) +
    geom_path() +
    geom_point() +
    scale_x_continuous(trans = "log2", breaks = c(8, 16, 32, 64, 128, 256, 512, 1024)) +
    scale_y_continuous(trans = "log", labels = scientific) +
    scale_color_discrete(labels = color_labs, breaks = breaks_order) +
    labs(y = "Time log(s)", x = "Size of Matrix", color = "Operation") +
    my_theme

# graph_sizes_tree <- ggplot(data = sizes_tree, aes(x = size, y = value, color = variable)) +
#     geom_path() +
#     geom_point() +
#     scale_x_continuous(trans = "log2", breaks = c(4, 8, 16, 32, 64, 128, 256, 512, 1024)) +
#     scale_y_continuous(labels = scientific) +
#     scale_color_discrete(labels = color_labs, breaks = breaks_order) +
#     labs(y = "Time (s)", x = "Size of Matrix", color = "Operation") +
#     my_theme

pdf("graphs.pdf", width = 10, height = 10)
print(graph_threads)
print(graph_threads_crit_tree)
# print(graph_threads_tree)
print(graph_sizes)
print(graph_sizes_crit_tree)
# print(graph_sizes_tree)
dev.off()
