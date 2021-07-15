library("plyr")
library("dplyr")
library("ggplot2")
library("xkcd")
library("patchwork")

pattern_rgb = function(p,i,lab) {
  pat = read.csv(paste(p,"REP_0/pass_through/twoDio/pattern_",i,".csv",sep=""))
  colnames(pat) = c("x","y","z")
  ggplot(data=pat,aes(x=x,y=y)) + geom_point(aes(col=z)) + scale_color_gradientn(colors=rainbow(3)) + ggtitle(lab)
}

spread = function(pat,i) {
 ( pat(exps[i],8,labels[i]) + pat(exps[i],9,labels[i]) ) /
 ( pat(exps[i],10,labels[i]) + pat(exps[i],11,labels[i]) ) 
}

#1d
#exps = list("qst/data/6425198615710403987_11234655969506889990/","qst/data/3259085005178809322_11234655969506889990/","qst/data/14625536848661304854_11234655969506889990/","qst/data/10186625217814687725_11234655969506889990/","qst/data/4906778666509337504_11234655969506889990/","qst/data/15598228468355886229_11234655969506889990/","qst/data/10790121203643274725_11234655969506889990/","qst/data/16461997677190402868_11234655969506889990/","qst/data/13814879895883276077_11234655969506889990/","qst/data/5785834293417641265_11234655969506889990/")
#labels = c("hiddens=0 ","hiddens=1 ","hiddens=2 ","hiddens=3 ","hiddens=4 ","hiddens=5 ","hiddens=6 ","hiddens=7 ","hiddens=8 ","hiddens=9 ")


spread(pattern_rgb,10)

p

i = seq(-6,6,by=0.1)
sins = sin(i)
df = data.frame(i = i, s = sins)
ggplot(df,aes(x=i)) + geom_line(aes(y=tan(i)))

source("anal.R")

