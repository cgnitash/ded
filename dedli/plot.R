
library("plyr")
library("dplyr")
library("ggplot2")
library("xkcd")
#library("patchwork")

summarize_population = function(path,i) {
  d = read.csv(path)
  c(i,mean(d$food_eaten),max(d$food_eaten),min(d$food_eaten))
}

aggregate_population = function(path,cycle_range,recorded_tag) {
 dfs = lapply(cycle_range, function(i) summarize_population(
  paste(path,recorded_tag,"_",i,".csv",sep=""),i))
 t(as.data.frame(dfs,row.names = c("update","avg","max","min")))
}


across_reps = function(file,component_name,recorded_tag,rep_range,cycle_range) {
  paths = lapply(rep_range, function(i) paste(file,i,component_name,"/",sep=""))
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
  for(i in 1:(length(labels))) {
    agg_exps <- merge(agg_exps, agg_datas[[i]], all = TRUE, 
                      suffixes = c("",labels[[i]]), by = "update")
  }
  
  list(agg_exps[-seq(2,6)], xrange = range(lapply(agg_datas, function(d) d$update)),
       yrange = range(lapply(agg_datas, function(d) d$mean)))
}

compute_all_oldv = function(stats,exps,labels,un_reported_data) {
 stats = enquo(stats)
 agg_datas = lapply(un_reported_data, function(e) report(!!stats,e))

 agg_exps = agg_datas[[1]]
   for(i in 1:(length(labels)-1)) {
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

cluster_plots = function(all_exps,ylabel,labs,indices,cols,cap) {
  exps = all_exps[[1]]
  xrange = all_exps[[2]]
  yrange = all_exps[[3]]
  
  master_plot = ggplot(data=exps, aes(x=update)) +
   scale_colour_manual("", 
                       breaks = lapply(indices, function(i) labs[i]),
                       values = cols) +
    labs(caption = cap,
         x = "update", y = ylabel) +
   theme_xkcd() +  
   xkcdaxis(xrange,yrange) 
  
  for(i in indices) {
   master_plot = master_plot + single_plot(exps,labs[i])
  }
  master_plot
}

extract_row = function(row,col) {
  mean = paste("mean",col,sep="")
  se = paste("se",col,sep="")
  c(col,row[[mean]],row[[mean]]+row[[se]],row[[mean]]-row[[se]])
}

final_fitness_plots = function(all_exps,ylabel,x_labs) {
  all_exps = all_avg
  exps = all_exps[[1]]
  last_row = tail(exps,1)
  
  x_labs = labels
  
  ddf = t(as.data.frame(lapply(x_labs, function(l) extract_row(last_row,l))))
  rownames(ddf) = x_labs
  colnames(ddf) = c("label","mean","sep","sem")
  
  ddf = as.data.frame(ddf)
  ddf$mean = as.numeric(as.character(ddf$mean))
  ddf$sem = as.numeric(as.character(ddf$sem))
  ddf$sep = as.numeric(as.character(ddf$sep))
  xrange = range(seq(1,length(x_labs)))
  yrange = range(min(as.numeric(ddf$mean)),max(as.numeric(ddf$mean)))
  
  ylabel = "finav"
  ggplot(data=ddf,aes(x=label)) + 
    geom_line(aes(y=mean,group=1),color="red") +
    geom_line(aes(y=sep,group=1),color="green") +
    geom_line(aes(y=sem,group=1),color="blue") +
    ylab(ylabel) +
    theme(axis.text.x=element_text(angle=40, hjust=1)) +
    theme_xkcd() 
}

# --------


#exps = list("qst/data/12688761706545524501_8585170111640645107/",
#            "qst/data/12688761706545524501_17578986486117912121/",
#            "qst/data/14292387654137485179_8585170111640645107/",
#            "qst/data/14292387654137485179_17578986486117912121/")

#labels = c("size=50 replace=true", 
#            "size=50 replace=false", 
#            "size=100 replace=true", 
#            "size=100 replace=false")

#source("anal.R")
#exps = list("data/13622521537752480560/", "data/1571991371793291181/","data/1979364337868570630/","data/2112679148773337681/","data/4511916868046805703/","data/11362886132779152776/")
#labels = c("hiddens = 2, strength = 0.25","hiddens = 2, strength = 0.5","hiddens = 3, strength = 0.25","hiddens = 3, strength = 0.5","hiddens = 4, strength = 0.25","hiddens = 4, strength = 0.5")

#extracted_exps = across_reps(e,component,tag,reps,cycles)

#  VERY expensive
#unr_data = un_reported_data(exps,"/_cycle/world_sequence/first_forager","food_eaten",0:4,seq(5,500,5))

#  VERY expensive
#for_unr_data = un_reported_data(exps,"two_cycle","score",0:19,0:300)

#all_max = compute_all(max,exps,labels,unr_data)

#all_avg = compute_all(avg,exps,labels,for_unr_data)

#all_avg = compute_all(avg,exps,labels,unr_data)

#exps
#labels

#pdf("pic.pdf")
#cluster_plots(all_avg,"avg",labels, c(1,2,3,4,5,6), palette(rainbow(7)), "qst/test1.exp")
#final_fitness_plots(all_avg,"Final avg",labels)
#dev.off()


#p2 = cluster_plots(all_avg,"avg",labels, 5:7, palette(rainbow(4)))
#p3 = cluster_plots(all_avg,"avg",labels, 8:10, palette(rainbow(4)))

#p = cluster_plots(all_avg,"avg",labels, 1:10, palette(rainbow(11)))
#p

#f1 = final_fitness_plots(all_avg,"Final avg",labels)
#f1

#p1 / p2

#x =palette(rainbow(5))
#p2 = cluster_plots(all_avg,"avg",labels,c(1,2),c("red", "green")) 
  
#p2

#p3 = cluster_plots(all_exps,"avg",labels,c(1,2,3,4),c("yellow", "blue","red", "green")) 

#p3

#( p1 / p2)
  
#p1 / p2

#vignette("xkcd-intro")
