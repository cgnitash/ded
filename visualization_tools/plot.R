
library("plyr")
library("dplyr")
library("ggplot2")
library("xkcd")
library("patchwork")

across_reps = function(file,rep_range) {
  files = lapply(rep_range, function(i) paste(file,"REP_",i,"/two_cycle/pop.csv",sep=""))
  plyr::ldply(lapply(files,read.csv),rbind)
}

report = function(var,data) {
  var = enquo(var)
  data %>% 
  dplyr::group_by(update) %>%
  dplyr::summarise(
      N = n(),
      mean = mean(!! var),
      median = median(!! var),
      sd  =  sd(!! var),
      se   = sd / sqrt(N)
  )
}

exp1 = "qst/data/12688761706545524501_8585170111640645107/"
exp2 = "qst/data/12688761706545524501_17578986486117912121/"
exp3 = "qst/data/14292387654137485179_8585170111640645107/"
exp4 = "qst/data/14292387654137485179_17578986486117912121/"

agg_data1 = report(avg,across_reps(exp1,0:9))
agg_data2 = report(avg,across_reps(exp2,0:9))
agg_data3 = report(avg,across_reps(exp3,0:9))
agg_data4 = report(avg,across_reps(exp4,0:9))

xrange = range(agg_data1$update)
yrange = range(agg_data1$mean)

l = list(agg_data1,agg_data2,agg_data3,agg_data4)
sfx <- c("_1", "_2", "_3", "_4")
res <- l[[1]]
for(i in head(seq_along(l), -1)) {
  res <- merge(res, l[[i+1]], all = TRUE, 
               suffixes = sfx[i:(i+1)], by = "update")
}

p1 = ggplot(data=res, aes(x=update)) + 
  geom_errorbar(aes(ymin=mean_1 - se_1, ymax=mean_1 + se_1, color="_1"),width=.1)  + 
  geom_line(aes(y=mean_1)) + 
  geom_errorbar(aes(ymin=mean_2 - se_2, ymax=mean_2 + se_2, color="_2"),width=.1)  + 
  geom_line(aes(y=mean_2)) + 
  scale_colour_manual("", 
                      breaks = c("_1", "_2"),
                      values = c("lightgoldenrod1", "mediumaquamarine")) +
  xlab("update") + 
  ylab("avg") +
  theme_xkcd() +  
  xkcdaxis(xrange,yrange) 

p2 = ggplot(data=res, aes(x=update)) + 
  geom_errorbar(aes(ymin=mean_3 - se_3, ymax=mean_3 + se_3, color="_3"),width=.1)  + 
  geom_line(aes(y=mean_3)) + 
  geom_errorbar(aes(ymin=mean_4 - se_4, ymax=mean_4 + se_4, color="_4"),width=.1)  + 
  geom_line(aes(y=mean_4)) + 
  scale_colour_manual("", 
                      breaks = c("_3", "_4"),
                      values = c("plum1", "skyblue1")) +
  xlab("update") + 
  ylab("avg") +
  theme_xkcd() +  
  xkcdaxis(xrange,yrange) 

p1 / p2

vignette("xkcd-intro")
