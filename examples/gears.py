#!/usr/bin/env python2.3
# * 3-D gear wheels.  This program is in the public domain.
# * Brian Paul
# * Conversion to GLUT by Mark J. Kilgard 
# conversion to Python using PyOpenGL with frame rates ala glxgears
# Peter Barth
# Converted to Python for S60/OpenGL ES 1.0 by Teemu Haapoja

import pygame
from pygame.locals import *
import sys
import egl
from gles import *
import time
from math import sin,cos,sqrt,pi

def float2fixed(values):
  ret = tuple([int(v*pow(2,16)) for v in values])
  return ret

class Gears:
  def __init__(self):
    # It's best to set these before creating the GL Canvas
    self.frames = 0
    self.exitflag = False
    self.render=0
    self.canvas = None
    self.angle = 0.0
    self.tStart = self.t0 = time.time()
    self.view_rotx=20.0
    self.view_roty=30.0
    self.view_rotz=0.0
    
    # Attributes for an antialiased canvas
    #egl_attrs = {EGL_SAMPLE_BUFFERS:1, EGL_SAMPLES:4}
    
    #appuifw.app.screen = 'full'
    try:
        pygame.init()
        self.screen = pygame.display.set_mode( (800,480), FULLSCREEN )
        egl.create(pygame.display.get_wm_info()['window'])
    except Exception,e:
        self.set_exit()
        return
    
    # The canvas is created, we are now ready to use GL functions
    print "GL_RENDERER   = %s" % (glGetString(GL_RENDERER))
    print "GL_VERSION    = %s" % (glGetString(GL_VERSION))
    print "GL_VENDOR     = %s" % (glGetString(GL_VENDOR))
    print "GL_EXTENSIONS = %s" % (glGetString(GL_EXTENSIONS))
    
    
    try:
      self.initgl()
      self.resize()
    except Exception,e:
      self.set_exit()
      
  def event(self, ev):
      # Change view angle
      if ev['type'] is not appuifw.EEventKey:
        return
      k = ev['scancode']
      if k == EScancode1:
          self.view_rotz += 5.0
      elif k == EScancode3:
          self.view_rotz -= 5.0
      elif k == EScancodeUpArrow:
          self.view_rotx += 5.0
      elif k == EScancodeDownArrow:
          self.view_rotx -= 5.0
      elif k == EScancodeRightArrow:
          self.view_roty += 5.0
      elif k == EScancodeLeftArrow:
          self.view_roty -= 5.0
    
  def resize(self):
      glViewport(0, 0, 800, 480)
      aspect = float(800) / float(480)
      glMatrixMode( GL_PROJECTION )
      glLoadIdentity()
      glFrustumf( -1.0*aspect, 1.0*aspect, -1.0, 1.0, 5.0, 60.0 )
      
      glMatrixMode(GL_MODELVIEW)
      glLoadIdentity()
      glTranslatef(0.0, 0.0, -40.0)
      
  def initgl(self):
    """Initializes OpenGL ES and the vertex/normal/triangle data."""
    pos = (5.0, 5.0, 10.0, 0.0)
    red = (0.8, 0.1, 0.0, 1.0)
    green = (0.0, 0.8, 0.2, 1.0)
    blue = (0.2, 0.2, 1.0, 1.0)
    
    glLightfv(GL_LIGHT0, GL_POSITION, pos)
    glEnable(GL_CULL_FACE)
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glEnable(GL_DEPTH_TEST)
    glEnableClientState(GL_VERTEX_ARRAY)
    glEnableClientState(GL_NORMAL_ARRAY)
    
    # Generate the gears
    self.gear1 = self.gear( 1.0, 4.0, 1.0, 20, 0.7)
    self.gear2 = self.gear( 0.5, 2.0, 2.0, 10, 0.7)
    self.gear3 = self.gear( 1.3, 2.0, 0.5, 10, 0.7)
    
    # Convert the data
    self.gear1['vertices'] = array(GL_FIXED, 3, self.gear1['vertices'])
    self.gear1['normals'] = array(GL_FIXED, 3, self.gear1['normals'])
    self.gear1['indices'] = array(GL_UNSIGNED_SHORT, 3, self.gear1['indices'])
    
    self.gear2['vertices'] = array(GL_FIXED, 3, self.gear2['vertices'])
    self.gear2['normals'] = array(GL_FIXED, 3, self.gear2['normals'])
    self.gear2['indices'] = array(GL_UNSIGNED_SHORT, 3, self.gear2['indices'])
    
    self.gear3['vertices'] = array(GL_FIXED, 3, self.gear3['vertices'])
    self.gear3['normals'] = array(GL_FIXED, 3, self.gear3['normals'])
    self.gear3['indices'] = array(GL_UNSIGNED_SHORT, 3, self.gear3['indices'])
    
    glEnable(GL_NORMALIZE)
    glShadeModel(GL_SMOOTH)
    self.render=1
    
  def gear(self, inner_radius, outer_radius, width, teeth, tooth_depth):
    r0 = float(inner_radius)
    r1 = float(outer_radius) - tooth_depth/2.0
    r2 = float(outer_radius) + tooth_depth/2.0
    da = 2.0*pi / teeth / 4.0
    
    icount = 0
    vertices = []
    indices = []
    normals = []
    
    normal = float2fixed((0.0, 0.0, 1.0))
    # Front face
    for i in range(teeth):
        angle = i * 2.0 * pi / teeth
        angle_next = (i-1) * 2.0 * pi / teeth
        # Triangle 1
        v1 = (r0*cos(angle), r0*sin(angle), width*0.5)
        v2 = (r1*cos(angle), r1*sin(angle), width*0.5)
        v3 = (r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [int(icount), icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        # Triangle 2
        v1 = (r0*cos(angle), r0*sin(angle), width*0.5)
        v2 = (r1*cos(angle_next+3*da), r1*sin(angle_next+3*da), width*0.5)
        v3 = (r1*cos(angle), r1*sin(angle), width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [int(icount), icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        # Triangle 3
        v1 = (r0*cos(angle), r0*sin(angle), width*0.5)
        v2 = (r0*cos(angle_next), r0*sin(angle_next), width*0.5)
        v3 = (r1*cos(angle_next+3*da), r1*sin(angle_next+3*da), width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [int(icount), icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
    
    # Front sides of teeth
    da = 2.0*pi / teeth / 4.0
    normal = float2fixed((0.0, 0.0, 1.0))
    for i in range(teeth):
        angle = i * 2.0*pi / teeth
        
        v1 = (r1*cos(angle),      r1*sin(angle),      width*0.5)
        v2 = (r2*cos(angle+da),   r2*sin(angle+da),   width*0.5)
        v3 = (r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [int(icount), icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = (r1*cos(angle),      r1*sin(angle),      width*0.5)
        v2 = (r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5)
        v3 = (r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [int(icount), icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
    normal = float2fixed((0.0, 0.0, -1.0))
    
    # Back face
    for i in range(teeth + 1):
        angle = i * 2.0*pi / teeth
        angle_next = (i+1) * 2.0*pi / teeth
        v1 = (r1*cos(angle), r1*sin(angle), -width*0.5)
        v2 = (r0*cos(angle), r0*sin(angle), -width*0.5)
        v3 = (r1*cos(angle+3*da), r1*sin(angle+3*da),-width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = (r0*cos(angle), r0*sin(angle), -width*0.5)
        v2 = (r0*cos(angle_next), r0*sin(angle_next), -width*0.5)
        v3 = (r1*cos(angle+3*da), r1*sin(angle+3*da),-width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = (r1*cos(angle+3*da), r1*sin(angle+3*da),-width*0.5)
        v2 = (r0*cos(angle_next), r0*sin(angle_next), -width*0.5)
        v3 = (r1*cos(angle_next), r1*sin(angle_next), -width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
      
    # Back sides of teeth
    da = 2.0*pi / teeth / 4.0
    for i in range(teeth):
        angle = i * 2.0*pi / teeth        
        
        v1 = (r1*cos(angle+3*da), r1*sin(angle+3*da),-width*0.5)
        v2 = (r2*cos(angle+2*da), r2*sin(angle+2*da),-width*0.5)
        v3 = (r2*cos(angle+da),   r2*sin(angle+da),  -width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = (r1*cos(angle),      r1*sin(angle),     -width*0.5)
        v2 = (r1*cos(angle+3*da), r1*sin(angle+3*da),-width*0.5)
        v3 = (r2*cos(angle+da),   r2*sin(angle+da),  -width*0.5)
        vertices.append( float2fixed(v1) )
        vertices.append( float2fixed(v2) )
        vertices.append( float2fixed(v3) )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
    
    # Outward faces of teeth
    angle = 0 * 2.0*pi / teeth
    normal = float2fixed((cos(angle), sin(angle), 0.0))
    for i in range(teeth):
        angle = i * 2.0*pi / teeth
        angle_next = (i-1) * 2.0*pi / teeth
        
        # # # Right side of a teeth
        v1 = float2fixed((r1*cos(angle), r1*sin(angle),  width*0.5))
        v2 = float2fixed((r1*cos(angle), r1*sin(angle), -width*0.5))
        u = r2*cos(angle+da) - r1*cos(angle)
        v = r2*sin(angle+da) - r1*sin(angle)
        len = sqrt(u*u + v*v)
        u = u / len
        v = v / len
        
        v3 = float2fixed((r2*cos(angle+da),   r2*sin(angle+da),   width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = float2fixed((r2*cos(angle+da),   r2*sin(angle+da),   width*0.5))
        v2 = float2fixed((r1*cos(angle), r1*sin(angle), -width*0.5))
        v3 = float2fixed((r2*cos(angle+da),   r2*sin(angle+da),  -width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        normal = float2fixed((v, -u, 0.0))
        
        # # # Left side of a teeth
        normal = float2fixed((cos(angle), sin(angle), 0.0))
        v1 = float2fixed((r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5))
        v2 = float2fixed((r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5))
        v3 = float2fixed((r2*cos(angle+2*da), r2*sin(angle+2*da),-width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        u = r1*cos(angle+3*da) - r2*cos(angle+2*da)
        v = r1*sin(angle+3*da) - r2*sin(angle+2*da)
        
        v1 = float2fixed((r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5))
        v2 = float2fixed((r2*cos(angle+2*da), r2*sin(angle+2*da),-width*0.5))
        v3 = float2fixed((r1*cos(angle+3*da), r1*sin(angle+3*da),-width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        normal = float2fixed((v, -u, 0.0))
        
        # # # Top of a teeth
        v1 = float2fixed((r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5))
        v2 = float2fixed((r2*cos(angle+da),   r2*sin(angle+da),   width*0.5))
        v3 = float2fixed((r2*cos(angle+da),   r2*sin(angle+da),  -width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = float2fixed((r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5))
        v2 = float2fixed((r2*cos(angle+da),   r2*sin(angle+da),  -width*0.5))
        v3 = float2fixed((r2*cos(angle+2*da), r2*sin(angle+2*da),-width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        # # # Bottom of a teeth
        v1 = float2fixed((r1*cos(angle), r1*sin(angle),  width*0.5))
        v2 = float2fixed((r1*cos(angle_next+3*da), r1*sin(angle_next+3*da), width*0.5))
        v3 = float2fixed((r1*cos(angle_next+3*da), r1*sin(angle_next+3*da), -width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        
        v1 = float2fixed((r1*cos(angle), r1*sin(angle),  width*0.5))
        v2 = float2fixed((r1*cos(angle_next+3*da), r1*sin(angle_next+3*da), -width*0.5))
        v3 = float2fixed((r1*cos(angle), r1*sin(angle), -width*0.5))
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal) )
        icount += 3
        normal = float2fixed((cos(angle), sin(angle), 0.0))
        
    # Inside radius cylinder
    for i in range(teeth + 1):
        angle = i * 2.0*pi / teeth;
        angle_next = (i+1) * 2.0*pi / teeth
        normal = float2fixed((-cos(angle), -sin(angle), 0.0))
        normal_next = float2fixed((-cos(angle_next), -sin(angle_next), 0.0))
        v1 = float2fixed((r0*cos(angle), r0*sin(angle), -width*0.5)) # 1
        v2 = float2fixed((r0*cos(angle), r0*sin(angle), width*0.5)) # 2
        v3 = float2fixed((r0*cos(angle_next), r0*sin(angle_next), width*0.5)) # 2b
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal,normal_next) )
        icount += 3
        
        v1 = float2fixed((r0*cos(angle), r0*sin(angle), -width*0.5)) # 1
        v2 = float2fixed((r0*cos(angle_next), r0*sin(angle_next), width*0.5)) # 2b
        v3 = float2fixed((r0*cos(angle_next), r0*sin(angle_next), -width*0.5)) # 1b
        vertices.append( v1 )
        vertices.append( v2 )
        vertices.append( v3 )
        
        indices.append( [icount, icount+1, icount+2] )
        normals.append( (normal,normal_next,normal_next) )
        icount += 3
    
    return {'vertices':vertices, 'indices':indices, 'normals':normals}
    
  def framerate(self):
      t = time.time()
      self.frames += 1
      if t - self.t0 >= 5.0:
          seconds = t - self.t0
          self.fps = self.frames/seconds
          print "%.0f frames in %3.1f seconds = %6.3f FPS" % (self.frames,seconds,self.fps)
          self.t0 = t
          self.frames = 0
    
  def redraw(self, frame):
    
    red = (0.8, 0.1, 0.0, 1.0)
    green = (0.0, 0.8, 0.2, 1.0)
    blue = (0.2, 0.2, 1.0, 1.0)
    self.angle += 2.0
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    #glLoadIdentity()
    #glTranslatef( 0.0, 0.0, -5.0 )
    #glScalef( 0.2, 0.2, 0.2)
    
    glPushMatrix()
    glRotatef(self.view_rotx, 1.0, 0.0, 0.0)
    glRotatef(self.view_roty, 0.0, 1.0, 0.0)
    glRotatef(self.view_rotz, 0.0, 0.0, 1.0)
    
    # Draw gear 1
    glPushMatrix()
    glTranslatef(-3.0, -2.0, 0.0)
    glRotatef(self.angle, 0.0, 0.0, 1.0)
    
    glVertexPointerx( self.gear1['vertices'] )
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, red)
    
    glNormalPointerx( self.gear1['normals']  )
    glDrawElementsus(GL_TRIANGLES, self.gear1['indices'])
    glPopMatrix()
    
    # Draw gear 2
    glPushMatrix()
    glTranslatef(3.1, -2.0, 0.0)
    glRotatef(-2.0*self.angle-9.0, 0.0, 0.0, 1.0)
    glVertexPointerx( self.gear2['vertices'] )
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, green)
    
    glNormalPointerx( self.gear2['normals']  )
    glDrawElementsus(GL_TRIANGLES, self.gear2['indices'])
    glPopMatrix()
    
    # Draw gear 3
    glPushMatrix()
    glTranslatef(-3.1, 4.2, 0.0)
    glRotatef(-2.0*self.angle-25.0, 0.0, 0.0, 1.0)
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blue)
    glVertexPointerx( self.gear3['vertices'] )
    
    glNormalPointerx( self.gear3['normals']  )
    glDrawElementsus(GL_TRIANGLES, self.gear3['indices'])
    
    glPopMatrix()
    
    glPopMatrix()
    
    self.framerate()
    
  def set_exit(self):
    self.exitflag = True
    
  def run(self):
    #appuifw.app.exit_key_handler=self.set_exit
    frame = 0
    while not self.exitflag:
        event = pygame.event.poll()
        if event:
            if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
                self.set_exit()
        self.redraw(frame)
        egl.swapbuffers()

if __name__ == '__main__':
    app=Gears()
    app.run()
    del app
