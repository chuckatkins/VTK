catch {load vtktcl}
if { [catch {set VTK_TCL $env(VTK_TCL)}] != 0} { set VTK_TCL "../../examplesTcl" }
if { [catch {set VTK_DATA $env(VTK_DATA)}] != 0} { set VTK_DATA "../../../vtkdata" }

# This example demonstrates how to set up flexible joints using 
# transform concatenation and vtkActor::SetUserTransform.
# The methods demonstrated here are preferable to those demonstrated
# in TransformConcatenation1.tcl.  

# You can use vtkAssembly to do the same thing, but this approach provides
# hands-on control over the orientation/position of each part relative to 
# the others i.e. you are not restricted to using vtkProp3D's methods for
# modifying the transform.

# create a rendering window and renderer
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin


# set up first set of polydata
vtkCylinderSource c1
c1 SetHeight 1.6
c1 SetRadius 0.2
c1 SetCenter 0 0.8 0

vtkTransform t1

vtkDataSetMapper m1
m1 SetInput [c1 GetOutput]

vtkActor a1
a1 SetUserTransform t1
a1 SetMapper m1
[a1 GetProperty] SetColor 1 0 0


# set up second set, at a relative transform to the first
vtkCylinderSource c2
c2 SetHeight 1.6
c2 SetRadius 0.15
c2 SetCenter 0 0.8 0

# relative translation for first joint
vtkTransform t2t
t2t Translate 0 1.6 0  
# relative rotation for first joint
vtkTransform t2r

# concatenate with initial transform
vtkLinearTransformConcatenation t2
t2 Concatenate t1
t2 Concatenate t2t
t2 Concatenate t2r

vtkDataSetMapper m2
m2 SetInput [c2 GetOutput]

vtkActor a2
a2 SetUserTransform t2
a2 SetMapper m2
[a2 GetProperty] SetColor 0.0 0.7 1.0


# set up third set, at a relative transform to the second
vtkCylinderSource c3
c3 SetHeight 0.5
c3 SetRadius 0.1
c3 SetCenter 0 0.25 0

# relative translation
vtkTransform t3t
t3t Translate 0 1.6 0
# relative rotation
vtkTransform t3r

# concatenate with previous segment
vtkLinearTransformConcatenation t3
t3 Concatenate t2
t3 Concatenate t3t
t3 Concatenate t3r

vtkDataSetMapper m3
m3 SetInput [c3 GetOutput]

vtkActor a3
a3 SetUserTransform t3
a3 SetMapper m3
[a3 GetProperty] SetColor 0.9 0.9 0

# combine actors into an assembly
# (this is just an option - you don't have to use an assembly)
# vtkAssembly assembly
# assembly AddPart a1
# assembly AddPart a2
# assembly AddPart a3

# add to renderer
# ren1 AddActor assembly

# You can add actors individually instead of using an assembly
ren1 AddActor a1
ren1 AddActor a2
ren1 AddActor a3

# set clipping range
ren1 ResetCamera -1 1 -0.1 2 -3 3

# enable user interface interactor
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize

# set angles for first joint
set phi2   70
set theta2 85
t2r RotateY $phi2
t2r RotateX $theta2

# set angles for second joint
set phi3   50
set theta3 90
t3r RotateY $phi3
t3r RotateX $theta3

global phi2 theta2 phi3 theta3

# create a control window

frame .w

frame .w.c2
label .w.c2.l2 -text "Joint #1"
scale .w.c2.phi2 -from -180 -to 180 -orient horizontal \
	-command SetAngles2 -variable phi2
scale .w.c2.theta2 -from -90 -to 90 -orient horizontal \
	-command SetAngles2 -variable theta2
pack .w.c2.l2 .w.c2.phi2 .w.c2.theta2 -side top
pack .w.c2 -side left

frame .w.c3
label .w.c3.l3 -text "Joint #2"
scale .w.c3.phi3 -from -180 -to 180 -orient horizontal \
	-command SetAngles3 -variable phi3
scale .w.c3.theta3 -from -90 -to 90 -orient horizontal \
	-command SetAngles3 -variable theta3
pack .w.c3.l3 .w.c3.phi3 .w.c3.theta3 -side top
pack .w.c3 -side left

pack .w

frame .ex
button .ex.exit -text "Exit" -command "exit"
pack .ex.exit -side top
pack .ex -side top

# create control procedures

proc SetAngles2 { dummy } {
    global phi2 theta2

    t2r Identity
    t2r RotateY $phi2
    t2r RotateX $theta2

    renWin Render
}    

proc SetAngles3 { dummy } {
    global phi3 theta3

    t3r Identity
    t3r RotateY $phi3
    t3r RotateX $theta3

    renWin Render
}    

