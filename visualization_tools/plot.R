
# read csv and add update column
av_data = read.csv(paste("data_",2,".csv",sep=""))
p = ggplot(data=av_data) + geom_line(aes(x=update,y=avg)) + geom_line(aes(x=update,y=max)) + xlab("update") + ylab("score")
p

across_reps = function(range) {
  files = lapply(range, function(i) paste("data/data_",i,"/pop.csv",sep=""))
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

agg_data = report(avg,across_reps(10:13))

p = ggplot(data=agg_data,aes(x=update,y=mean)) + geom_errorbar(aes(ymin=mean-se, ymax=mean+se), color="gray",width=.01)  + geom_line() + xlab("update") + ylab("fitness") 
p
