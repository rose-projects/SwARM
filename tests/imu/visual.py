#!/usr/bin/env python
import plotly
import plotly.plotly as py
import pandas as pd

df = pd.read_csv('data.csv')
df.head()

scatter = dict(
    mode = "markers",
    name = "y",
    type = "scatter3d",    
    x = df['x'], y = df['y'], z = df['z'],
    marker = dict( size=2, color="rgb(23, 190, 207)" )
)
clusters = dict(
    alphahull = 7,
    name = "y",
    opacity = 0.1,
    type = "mesh3d",    
    x = df['x'], y = df['y'], z = df['z']
)
layout = dict(
    title = '3d point clustering',
    scene = dict(
        xaxis = dict( zeroline=False ),
        yaxis = dict( zeroline=False ),
        zaxis = dict( zeroline=False ),
    )
)
fig = dict( data=[scatter, clusters], layout=layout )
# Use py.iplot() for IPython notebook
plotly.offline.plot(fig, filename='3d point clustering')