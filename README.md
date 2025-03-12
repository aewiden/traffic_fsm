A finite state machine that controls a railway crossing. The crossing consists of a one-way road with 
an associated set of traffic lights, a blue maintenance light, a railway track with a gate,
and a pedestrian crossing with two associated push buttons. Each crossing has a unique id.
Generally, when there are no trains, traffic is allowed to flow continuously for a
minimum of three minutes. If a pedestrian pushes a crossing button, only after the
three minutes are up and the red light is illuminated, pedestrians are given 30
seconds to cross the road. The traffic lights sequence on a 3 second interval between
different colors.
The crossing communicates with a railway switching substation that holds a database
of crossings and their status. When a train is arriving, the substation sends an “train
arriving” message to the controller. The controller stops traffic flow and closes the
gate. The substation monitors the railway line and informs the controller when the
train has passed with a “clear” message; the controller then raises the gate, waits 20
seconds to ensure there are no pedestrians on the crosswalk, and allows traffic to
flow with the appropriate traffic signals. While the traffic is stopped, pedestrians can
freely cross the road.
Occasionally, a railway engineer may walk up to the crossing, insert a key, and
manually close the crossing for maintenance; in this mode trains may still pass, but
the engineer may work on the crossing hardware without traffic flowing. When the
key is turned, the controller stops the traffic, closes the gate, and flashes a blue light
on the gate at 1 second intervals. A wheel at the side of the road can be used to
manually raise and lower the gate. The engineer may manually initiate opening the
crossing by use of his key.
