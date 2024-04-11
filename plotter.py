#!/usr/bin/python
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
#from pathlib import Path
import os

data = pd.read_csv("xlines.csv",delimiter=";")

mypathI= os.path.getsize('inter.csv')
mypathC= os.path.getsize('centroid.csv')

if (mypathI!=0):
    df= pd.read_csv("inter.csv", delimiter=";")
if (mypathC!=0):
    df_clus = pd.read_csv("centroid.csv",delimiter=';')

clist = ['red','pink','purple','blue','cyan','green','olive','orange','brown','black','gray','grey','yellow',
         'silver','tan','navy','lightpink','peru']
for idx in range(data.index.stop) :
    color='black'
    way=data.track[idx]
    
    if way[0]=='B' :
        tcolor='blue'
    elif way[0]=='R':
        tcolor='red'
    elif way[0]=='Y':
        tcolor='gold'
        
    line0 = (eval(data.pt0[idx]),eval(data.pt1[idx]))
    xs,ys = zip(*line0)
    plt.plot(xs,ys,'--', markersize=0, color=tcolor, linewidth=.3)

if (mypathI!=0):
    for jdx in range(df.index.stop):
        px = df.x[jdx]
        py  = df.y[jdx]
        plt.scatter(px,py,2,color='black',marker='x')


if (mypathC!=0):
    colormap = plt.cm.gist_ncar #nipy_spectral, Set1,Paired   
    colors = [colormap(i) for i in np.linspace(0, 10,500)]
    for kdx in range(df_clus.index.stop):
        flagColor = df_clus.centroidFlag[kdx]
        if (flagColor==7) :
            cx = df_clus.x[kdx]
            cy = df_clus.y[kdx]
            ll = str(cx)+','+str(cy)
            #plt.scatter(cx,cy,facecolors='none',s= 100,label=ll,color=colors[kdx])
            #plt.scatter(cx,cy,s= 100,label=ll,color=colors[kdx])
            plt.scatter(cx,cy,s= 100,label=ll,color='dimgrey')
            #plt.scatter(cx,cy,facecolors='none',s= 100,label=ll,color='black')

plt.ylim(-4000,4000)
plt.xlim(-4000,5000)
plt.xlabel('X-axis [$\mu$m]')
plt.ylabel('Y-axis [$\mu$m]')
plt.title('PICMIC$0$ Intersections, Centroids and Clusters')
#plt.grid()
if (mypathC!=0):
    plt.legend( fontsize=9,bbox_to_anchor=(0.5, 0., 0.5, 0.5) )
#plt.axis('off')
plt.savefig('plot.png')
plt.show()

exit()

