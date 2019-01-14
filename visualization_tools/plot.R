
library("plyr")
library("dplyr")
library("ggplot2")
library("xkcd")
library("patchwork")

across_reps = function(file,rep_range) {
  files = lapply(rep_range, function(i) paste(file,"REP_",i,"/two_cycle/pop.csv",sep=""))
  ldply(lapply(files,read.csv),rbind)
}

report = function(var,data) {
  var = enquo(var)
  data %>% 
  group_by(update) %>%
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

p1 = ggplot(data=agg_data1,aes(x=update,y=mean)) + 
  geom_errorbar(aes(ymin=mean-se, ymax=mean+se), color="lightgoldenrod1",width=.1)  + 
  geom_line() + 
  xlab("update") + 
  ylab("fitness") +
  theme_xkcd() +  
  xkcdaxis(xrange,yrange) 

p2 = ggplot(data=agg_data2,aes(x=update,y=mean)) + 
  geom_errorbar(aes(ymin=mean-se, ymax=mean+se), color="lightgoldenrod1",width=.1)  + 
  geom_line() + 
  xlab("update") + 
  ylab("fitness") +
  theme_xkcd() +
  xkcdaxis(xrange,yrange) 

p3 = ggplot(data=agg_data3,aes(x=update,y=mean)) + 
  geom_errorbar(aes(ymin=mean-se, ymax=mean+se), color="lightgoldenrod1",width=.1)  + 
  geom_line() + 
  xlab("update") + 
  ylab("fitness") +
  theme_xkcd() +
  xkcdaxis(xrange,yrange) 

p4 = ggplot(data=agg_data4,aes(x=update,y=mean)) + 
  geom_errorbar(aes(ymin=mean-se, ymax=mean+se), color="lightgoldenrod1",width=.1)  + 
  geom_line() + 
  xlab("update") + 
  ylab("fitness") +
  theme_xkcd() +
  xkcdaxis(xrange,yrange) 

p1 + p2 + p3 + p4

p1 
p2
p3
p4

vignette("xkcd-intro")
