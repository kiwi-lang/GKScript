GKScript
=============

Experimental plugin to transform blueprint code into a python like textual representation.

The transform is implemented as a Commandlet & a Menu entry when right clicking on Blueprint.
The commandlet can be used to convert Blueprint in batch.

Goals
-----

Split blueprint into a data component (.uasset) and a textual code component (.us).
The code should be displayed in the editor as the usual graph but saved as its text representation.

* Blueprint Graph -> Text: WIP
* Text -> Blueprint Graph: Not started





Output Example
--------------

.. code-block:: python

   class BP_TopDownController(PlayerController):
      """PlayerController Blueprint handles clicking and telling Character where to go."""

      PressedThreshold: real = DefaultSubObject()
      FXCursor: NiagaraSystem = DefaultSubObject()
      CachedDestination: Vector = DefaultSubObject()

      # FunctionEntry
      def UserConstructionScript():
         """"""

      # FunctionEntry
      def MoveTo(Goal):
         """Description"""
         # CallFunction
         SimpleMoveToLocation(
            self = Object,
            Controller = self,
            Goal = Goal,
         )
         # CallFunction
         NiagaraComponent_1 = SpawnSystemAtLocation(
            self = Object,
            WorldContextObject = MissingLink(),
            SystemTemplate = FXCursor,
            Location = Goal,
            Rotation = (0, 0, 0),
            Scale = (1.000000,1.000000,1.000000),
            bAutoDestroy = True,
            bAutoActivate = True,
            PoolingMethod = None,
            bPreCullCheck = True,
         )

      # FunctionEntry
      def Follow(To):
         """"""
         # CallFunction
         Pawn_1 = K2_GetPawn(
            self = MissingLink(),
         )
         # CallFunction
         From = K2_GetActorLocation(
            self = Pawn_1,
         )
         # CallFunction
         WorldDirection = GetDirectionUnitVector(
            self = Object,
            From = From,
            To = To,
         )
         # CallFunction
         AddMovementInput(
            self = Pawn_1,
            WorldDirection = WorldDirection,
            ScaleValue = 1.000000,
            bForce = False,
         )

      # FunctionEntry
      def GetLocationUnderCursor():
         """"""
         # CallFunction
         Hit, bool_1 = GetHitResultUnderCursorByChannel(
            self = MissingLink(),
            TraceChannel = TraceTypeQuery1,
            bTraceComplex = True,
         )
         # CallFunction
         bBlockingHit, bInitialOverlap, Time, Distance, Location, ImpactPoint, Normal, ImpactNormal, PhysMat, HitActor, HitComponent, HitBoneName, BoneName, HitItem, ElementIndex, FaceIndex, TraceStart, TraceEnd = BreakHitResult(
            self = Object,
            Hit = Hit,
         )
         # FunctionResult
         Hit = bBlockingHit
         Location = Location
         return Hit, Location

      # FunctionEntry
      def GetLocationUnderFinger():
         """"""
         # CallFunction
         Hit, bool_1 = GetHitResultUnderFingerByChannel(
            self = MissingLink(),
            FingerIndex = Touch1,
            TraceChannel = TraceTypeQuery1,
            bTraceComplex = False,
         )
         # CallFunction
         bBlockingHit, bInitialOverlap, Time, Distance, Location, ImpactPoint, Normal, ImpactNormal, PhysMat, HitActor, HitComponent, HitBoneName, BoneName, HitItem, ElementIndex, FaceIndex, TraceStart, TraceEnd = BreakHitResult(
            self = Object,
            Hit = Hit,
         )
         # FunctionResult
         Hit = bBlockingHit
         Location = Location
         return Hit, Location

      # FunctionEntry
      def DummyTestFunction(Condition):
         """"""
         # IfThenElse
         if Condition:
            # CallFunction
            Hit, Location = GetLocationUnderCursor(
               self = MissingLink(),
            )
            # FunctionResult
            Hit = Hit
            Location = Location
            return Hit, Location
         else:
            # CallFunction
            Hit, Location = GetLocationUnderFinger(
               self = MissingLink(),
            )
            # FunctionResult
            Hit = Hit
            Location = Location
            return Hit, Location

      # Event
      def On_ReceiveBeginPlay():
         """Event when play begins for this actor.

         Target is Actor"""
         # MacroInstance
         GetSubsystem(=ClassName)    # Tunnel


.. code-block:: bash

   $ uecli gamekit gkscript --project GamekitDev
   [  0][V][LogGKScript               ]  Parameters:  -run=GKScript -NoLiveCoding -fullstdoutlogoutput -utf8output -nullrhi -nosplash -nosound -nopause -unattended
   [  0][V][LogGKScript               ]   - GKSCRIPT_TAG   : v0.0.0
   [  0][V][LogGKScript               ]   - GKSCRIPT_COMMIT: nohash
   [  0][V][LogGKScript               ]   - GKSCRIPT_DATE  : 1900-01-01 01:01:01 +0000
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  >> Generating Code
   [  0][V][LogGKScript               ]   - E:/GamekitDev/Content/GKScript/BP_TopDownController.us
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  +-> FunctionEntry
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  +-> FunctionEntry
   [  0][V][LogGKScript               ]  |+-> CallFunction
   [  0][V][LogGKScript               ]  |:+-> CallFunction
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  +-> FunctionEntry
   [  0][V][LogGKScript               ]  |+-> CallFunction
   [  0][V][LogGKScript               ]  |:+-> CallFunction
   [  0][V][LogGKScript               ]  |:+-> CallFunction
   [  0][V][LogGKScript               ]  |:|+-> CallFunction
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  +-> FunctionEntry
   [  0][V][LogGKScript               ]  |+-> FunctionResult
   [  0][V][LogGKScript               ]  |:+-> CallFunction
   [  0][V][LogGKScript               ]  |:|+-> CallFunction
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  +-> FunctionEntry
   [  0][V][LogGKScript               ]  |+-> FunctionResult
   [  0][V][LogGKScript               ]  |:+-> CallFunction
   [  0][V][LogGKScript               ]  |:|+-> CallFunction
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  +-> Event
   [  0][V][LogGKScript               ]  |+-> MacroInstance
   [  0][V][LogGKScript               ]  |:+-> GetSubsystem
   [  0][V][LogGKScript               ]  |:+-> Tunnel
   [  0][V][LogGKScript               ]
   [  0][V][LogGKScript               ]  << Finished
   [  0][V][LogGKScript               ]


Features
--------

* With UnrealEngine Editor

.. code-block::

   UnrealEditor-Cmd.exe E:/GamekitDev/GamekitDev.uproject -run=GKScript


Useful Links
------------

* `Marketplace <https://www.unrealengine.com/marketplace/en-US/product/>`_

