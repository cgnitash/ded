
# read csv and add update column
av_data = read.csv(paste("data_",2,".csv",sep=""))
p = ggplot(data=av_data) + geom_line(aes(x=update,y=avg)) + geom_line(aes(x=update,y=max)) + xlab("update") + ylab("score")
p

across_reps = function(prefix,range) {
  files = lapply(range, function(i) paste(prefix,i,".csv",sep=""))
  ldply(lapply(files,read.csv),rbind)
}

report = function(data, var) {
  var = enquo(var)
  data %>% 
  group_by(update) %>%
  summarise(
      N = n(),
      mean = mean(!! var),
      median = median(!! var),
      sd  =  sd(!! var),
      se   = sd / sqrt(N)
  )
}

agg_data = report(across_reps("data_",101:110),avg)

p = ggplot(data=agg_data,aes(x=update,y=mean)) + geom_errorbar(aes(ymin=mean-se, ymax=mean+se), width=.01)  + geom_line() + xlab("update") + ylab("fitness") 
p


