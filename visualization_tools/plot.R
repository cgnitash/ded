
library("plyr")
library("dplyr")
library("ggplot2")
library("xkcd")
library("patchwork")

summarize_population = function(path,i) {
  d = read.csv(path)
  c(i,mean(d$score),max(d$score),min(d$score))
}

aggregate_population = function(path,cycle_range,recorded_tag) {
 dfs = lapply(cycle_range, function(i) summarize_population(
  paste(path,recorded_tag,"_",i,".csv",sep=""),i))
 t(as.data.frame(dfs,row.names = c("update","avg","max","min")))
}


across_reps = function(file,component_name,recorded_tag,rep_range,cycle_range) {
  paths = lapply(rep_range, function(i) paste(file,"REP_",i,"/",component_name,"/",sep=""))
  plyr::ldply(lapply(paths,aggregate_population,cycle_range=cycle_range,recorded_tag=recorded_tag),rbind)
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

compute_all = function(stats,exps,labels,un_reported_data) {
 stats = enquo(stats)
 agg_datas = lapply(un_reported_data, function(e) report(!!stats,e))

 agg_exps = agg_datas[[1]]
 for(i in head(seq_along(agg_datas), -1)) {
  agg_exps <- merge(agg_exps, agg_datas[[i+1]], all = TRUE, 
               suffixes = labels[i:(i+1)], by = "update")
 }
 
 list(agg_exps, xrange = range(lapply(agg_datas, function(d) d$update)),
             yrange = range(lapply(agg_datas, function(d) d$mean)))
}

single_plot = function(all_exps,col) {
  mean = paste("mean",col,sep="")
  se = paste("se",col,sep="")
  
  list(geom_errorbar(aes(ymin= !!ensym(mean) - !!ensym(se), ymax= !!ensym(mean) + !!ensym(se), color=col),alpha=0.4,width=.1),
    geom_line(aes(y=!!ensym(mean)))) 
}

un_reported_data = function(exps,component,tag,reps,cycles) {
  lapply(exps, function(e) across_reps(e,component,tag,reps,cycles))
}

cluster_plots = function(all_exps,ylabel,labs,indices,cols) {
  exps = all_exps[[1]]
  xrange = all_exps[[2]]
  yrange = all_exps[[3]]
  
  master_plot = ggplot(data=exps, aes(x=update)) +
   scale_colour_manual("", 
                       breaks = lapply(indices, function(i) labs[i]),
                       values = cols) +
   xlab("update") + 
   ylab(ylabel) +
   theme_xkcd() +  
   xkcdaxis(xrange,yrange) 
  
   for(i in indices) {
     master_plot = master_plot + single_plot(exps,labs[i])
   }
  master_plot
}

#exps = list("qst/data/12688761706545524501_8585170111640645107/",
#            "qst/data/12688761706545524501_17578986486117912121/",
#            "qst/data/14292387654137485179_8585170111640645107/",
#            "qst/data/14292387654137485179_17578986486117912121/")

#labels = c("size=50 replace=true", 
#            "size=50 replace=false", 
#            "size=100 replace=true", 
#            "size=100 replace=false")

                                            
#extracted_exps = across_reps(e,component,tag,reps,cycles)

#unr_data = un_reported_data(exps,"two_cycle","score",0:9,0:300)

#all_max = compute_all(max,exps,labels,unr_data)

#all_avg = compute_all(avg,exps,labels,unr_data)

#p1 = cluster_plots(all_max,"max",labels, c(1,2),c("yellow", "blue")) 

#p1 / p2

#p2 = cluster_plots(all_avg,"avg",labels,c(1,2),c("red", "green")) 
  
#p2

#p3 = cluster_plots(all_exps,"avg",labels,c(1,2,3,4),c("yellow", "blue","red", "green")) 

#p3

#( p1 / p2)
  
#p1 / p2

#vignette("xkcd-intro")
