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

Example
-------

.. code-block:: bash

   $ uecli gkscript --project GamekitDev
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

.. code-block:: python

   # FunctionEntry
   def UserConstructionScript():
      """"""

   # FunctionEntry
   def MoveTo(Goal: Vector):
      """Description"""
      # CallFunction
      SimpleMoveToLocation(
         self: AIBlueprintHelperLibrary = Object,
         Controller: Controller = self,
         Goal: Vector = Goal,
      )
      # CallFunction
      NiagaraComponent_1: NiagaraComponent = SpawnSystemAtLocation(
         self: NiagaraFunctionLibrary = Object,
         WorldContextObject: Object,
         SystemTemplate: NiagaraSystem = FXCursor,
         Location: Vector = Goal,
         Rotation: Rotator = (0, 0, 0),
         Scale: Vector = (1.000000,1.000000,1.000000),
         bAutoDestroy: bool = true,
         bAutoActivate: bool = true,
         PoolingMethod: ENCPoolMethod = None,
         bPreCullCheck: bool = true,
      )

   # FunctionEntry
   def Follow(To: Vector):
      """"""
      # CallFunction
      Pawn_1: Pawn = K2_GetPawn(
         self: Controller,
      )
      # CallFunction
      From: Vector = K2_GetActorLocation(
         self: Actor = Pawn_1,
      )
      # CallFunction
      WorldDirection: Vector = GetDirectionUnitVector(
         self: KismetMathLibrary = Object,
         From: Vector = From,
         To: Vector = To,
      )
      # CallFunction
      AddMovementInput(
         self: Pawn = Pawn_1,
         WorldDirection: Vector = WorldDirection,
         ScaleValue: real = 1.000000,
         bForce: bool = false,
      )

   # FunctionEntry
   def GetLocationUnderCursor():
      """"""
      # FunctionResult
      # CallFunction
      Hit: HitResult, bool_1: bool = GetHitResultUnderCursorByChannel(
         self: PlayerController,
         TraceChannel: ETraceTypeQuery = TraceTypeQuery1,
         bTraceComplex: bool = TRUE,
      )
      # CallFunction
      bBlockingHit: bool, bInitialOverlap: bool, Time: real, Distance: real, Location: Vector, ImpactPoint: Vector, Normal: Vector, ImpactNormal: Vector, PhysMat: PhysicalMaterial, HitActor: Actor, HitComponent: PrimitiveComponent, HitBoneName: name, BoneName: name, HitItem: int, ElementIndex: int, FaceIndex: int, TraceStart: Vector, TraceEnd: Vector = BreakHitResult(
         self: GameplayStatics = Object,
         Hit: HitResult = Hit,
      )
      return Hit: bool = bBlockingHit, Location: Vector = Location

   # FunctionEntry
   def GetLocationUnderFinger():
      """"""
      # FunctionResult
      # CallFunction
      Hit: HitResult, bool_1: bool = GetHitResultUnderFingerByChannel(
         self: PlayerController,
         FingerIndex: ETouchIndex = Touch1,
         TraceChannel: ETraceTypeQuery = TraceTypeQuery1,
         bTraceComplex: bool = false,
      )
      # CallFunction
      bBlockingHit: bool, bInitialOverlap: bool, Time: real, Distance: real, Location: Vector, ImpactPoint: Vector, Normal: Vector, ImpactNormal: Vector, PhysMat: PhysicalMaterial, HitActor: Actor, HitComponent: PrimitiveComponent, HitBoneName: name, BoneName: name, HitItem: int, ElementIndex: int, FaceIndex: int, TraceStart: Vector, TraceEnd: Vector = BreakHitResult(
         self: GameplayStatics = Object,
         Hit: HitResult = Hit,
      )
      return Hit: bool = bBlockingHit, Location: Vector = Location

      # Event
      def On_ReceiveBeginPlay():
      """Event when play begins for this actor.
      
      Target is Actor"""
      # MacroInstance
      GetSubsystem(=ClassName)



Features
--------

* With UnrealEngine Editor

.. code-block::

   UnrealEditor-Cmd.exe E:/GamekitDev/GamekitDev.uproject -run=GKScript


Useful Links
------------

* `Marketplace <https://www.unrealengine.com/marketplace/en-US/product/>`_

