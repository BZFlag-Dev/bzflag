Included here should be the core non-generic logic of bzflag.  The
aspects of the game that are potentially useful to other applications
(server, client, or otherwise) should be put here instead of into the
application front-ends.  The game lib is a middle layer sitting
between the generic common functionality that is completely
application-agnostic and the logic and appearance completely specific
to a single particular application.  This means that there should be
no explicit display code such as OpenGL calls.  In the
model-view-controller paradigm, this library is for the model and and
view-agnostic controllers.

If your code is not bz-specific or could not potentially be used in
another application, it probably doesn't belong in here -- it probably
belongs in the common library.  If your code cannot really be used in
another application without modification, perhaps specific or tied to
rendering (e.g. OpenGL calls) or display of data, it probably doesn't
belong in here.
