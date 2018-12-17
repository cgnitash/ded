
# read csv and add update column
library("plyr")
library("dplyr")
library("ggplot2")
library("xkcd")

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

agg_data = report(avg,across_reps(10:15))

xrange = range(agg_data$update)
yrange = range(agg_data$mean)

ratioxy <- diff(xrange)/diff(yrange)

mapping <- aes(x, y, scale, ratioxy, angleofspine, 
               anglerighthumerus, anglelefthumerus,
               anglerightradius, angleleftradius,
               anglerightleg, angleleftleg, angleofneck)

simple_dataman <- data.frame(x= 4*diff(xrange)/5, y= diff(yrange)/3,
                             scale = 0.8 ,
                             ratioxy = ratioxy,
                             angleofspine = -pi/2 ,
                             anglerighthumerus = -pi/6,
                             anglelefthumerus = pi - pi/6,
                             anglerightradius = -pi/3,
                             angleleftradius =  3*pi/4,
                             anglerightleg = 3*pi/2 - pi / 12,
                             angleleftleg = 3*pi/2 + pi / 12 ,
                             angleofneck = -pi + pi/2)

datalines <- data.frame(x=9*diff(xrange)/10,y=diff(yrange)/2.5,xend=9.1*diff(xrange)/10,yend=diff(yrange)/2.2)

p = ggplot(data=agg_data,aes(x=update,y=mean)) + 
  geom_errorbar(aes(ymin=mean-se, ymax=mean+se), color="lightgoldenrod1",width=.1)  + 
  geom_line() + 
  xlab("update") + 
  ylab("fitness") +
  theme_xkcd() +
  xkcdaxis(xrange,yrange) +
  xkcdman(mapping,simple_dataman) +
  annotate("text", x=9*diff(xrange)/10, y = diff(yrange)/2,
           label = "Whoa!\nNice Graph, bro!", family="xkcd" ) +
  xkcdline(aes(x=x,y=y,xend=xend,yend=yend), datalines,
            xjitteramount = 0.05)

p

vignette("xkcd-intro")
