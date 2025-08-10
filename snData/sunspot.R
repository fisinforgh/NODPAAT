iniYY = 1979
endYY = 2024

df<- read.csv2("/home/jaiver/Documents/Nasa_data/to3soft_chi2_Grids/sunspots/SN_d_tot_V2.0.csv", sep = ';',header = F)
head(df)

df <- type.convert(df, as.is = TRUE)

head(df)

df_skim<-df[(df$V1>=iniYY)&(df$V1<=endYY), ]

head(df_skim)

write.table(df_skim, sep = '\t', file = "sndataskim.dat", col.names=FALSE, row.names=FALSE)
write.table(df, sep = '\t', file = "sndata.dat", col.names=FALSE, row.names=FALSE)

system("mv sndataskim.dat ../snData/")
system("mv sndata.dat ../snData/")