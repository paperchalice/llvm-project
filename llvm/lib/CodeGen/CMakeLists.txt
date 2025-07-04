if (DEFINED LLVM_HAVE_TF_AOT OR LLVM_HAVE_TFLITE)
  include(TensorFlowCompile)
  set(LLVM_RAEVICT_MODEL_PATH_DEFAULT "models/regalloc-eviction")

  set(LLVM_RAEVICT_MODEL_CURRENT_URL "<UNSPECIFIED>" CACHE STRING "URL to download the LLVM register allocator eviction model")

  if (DEFINED LLVM_HAVE_TF_AOT)
    tf_find_and_compile(
      ${LLVM_RAEVICT_MODEL_PATH}
      ${LLVM_RAEVICT_MODEL_CURRENT_URL}
      ${LLVM_RAEVICT_MODEL_PATH_DEFAULT}
      "../Analysis/models/gen-regalloc-eviction-test-model.py"
      serve
      action
      RegAllocEvictModel
      llvm::RegAllocEvictModel
    )
  endif()

  if (LLVM_HAVE_TFLITE)
    list(APPEND MLLinkDeps ${tensorflow_c_api} ${tensorflow_fx})
  endif()
endif()

add_llvm_component_library(LLVMCodeGen
  AggressiveAntiDepBreaker.cpp
  AllocationOrder.cpp
  Analysis.cpp
  AssignmentTrackingAnalysis.cpp
  AtomicExpandPass.cpp
  BasicTargetTransformInfo.cpp
  BranchFolding.cpp
  BranchRelaxation.cpp
  BreakFalseDeps.cpp
  BasicBlockSections.cpp
  BasicBlockPathCloning.cpp
  BasicBlockSectionsProfileReader.cpp
  CalcSpillWeights.cpp
  CallBrPrepare.cpp
  CallingConvLower.cpp
  CFGuardLongjmp.cpp
  CFIFixup.cpp
  CFIInstrInserter.cpp
  CodeGen.cpp
  CodeGenCommonISel.cpp
  CodeGenPrepare.cpp
  CommandFlags.cpp
  ComplexDeinterleavingPass.cpp
  CriticalAntiDepBreaker.cpp
  DeadMachineInstructionElim.cpp
  DetectDeadLanes.cpp
  DFAPacketizer.cpp
  DroppedVariableStatsMIR.cpp
  DwarfEHPrepare.cpp
  EarlyIfConversion.cpp
  EdgeBundles.cpp
  EHContGuardTargets.cpp
  ExecutionDomainFix.cpp
  ExpandLargeDivRem.cpp
  ExpandFp.cpp
  ExpandMemCmp.cpp
  ExpandPostRAPseudos.cpp
  ExpandReductions.cpp
  ExpandVectorPredication.cpp
  FaultMaps.cpp
  FEntryInserter.cpp
  FinalizeISel.cpp
  FixupStatepointCallerSaved.cpp
  FuncletLayout.cpp
  MachineFunctionAnalysis.cpp
  GCMetadata.cpp
  GCMetadataPrinter.cpp
  GCRootLowering.cpp
  GlobalMerge.cpp
  GlobalMergeFunctions.cpp
  HardwareLoops.cpp
  IfConversion.cpp
  ImplicitNullChecks.cpp
  IndirectBrExpandPass.cpp
  InitUndef.cpp
  InlineSpiller.cpp
  InterferenceCache.cpp
  InterleavedAccessPass.cpp
  InterleavedLoadCombinePass.cpp
  IntrinsicLowering.cpp
  JMCInstrumenter.cpp
  KCFI.cpp
  LatencyPriorityQueue.cpp
  LazyMachineBlockFrequencyInfo.cpp
  LexicalScopes.cpp
  LiveDebugVariables.cpp
  LiveIntervals.cpp
  LiveInterval.cpp
  LiveIntervalUnion.cpp
  LivePhysRegs.cpp
  LiveRangeCalc.cpp
  LiveIntervalCalc.cpp
  LiveRangeEdit.cpp
  LiveRangeShrink.cpp
  LiveRegMatrix.cpp
  LiveRegUnits.cpp
  LiveStacks.cpp
  LiveVariables.cpp
  CodeGenTargetMachineImpl.cpp
  LocalStackSlotAllocation.cpp
  LoopTraversal.cpp
  LowLevelTypeUtils.cpp
  LowerEmuTLS.cpp
  MachineBasicBlock.cpp
  MachineBlockFrequencyInfo.cpp
  MachineBlockPlacement.cpp
  MachineBranchProbabilityInfo.cpp
  MachineCFGPrinter.cpp
  MachineCombiner.cpp
  MachineConvergenceVerifier.cpp
  MachineCopyPropagation.cpp
  MachineCSE.cpp
  MachineCheckDebugify.cpp
  MachineCycleAnalysis.cpp
  MachineDebugify.cpp
  MachineDomTreeUpdater.cpp
  MachineDominanceFrontier.cpp
  MachineDominators.cpp
  MachineFrameInfo.cpp
  MachineFunction.cpp
  MachineFunctionPass.cpp
  MachineFunctionPrinterPass.cpp
  MachineFunctionSplitter.cpp
  MachineInstrBundle.cpp
  MachineInstr.cpp
  MachineLateInstrsCleanup.cpp
  MachineLICM.cpp
  MachineLoopInfo.cpp
  MachineLoopUtils.cpp
  MachineModuleInfo.cpp
  MachineModuleInfoImpls.cpp
  MachineModuleSlotTracker.cpp
  MachineOperand.cpp
  MachineOptimizationRemarkEmitter.cpp
  MachineOutliner.cpp
  MachinePassManager.cpp
  MachinePipeliner.cpp
  MachinePostDominators.cpp
  MachineRegionInfo.cpp
  MachineRegisterInfo.cpp
  MachineScheduler.cpp
  MachineSink.cpp
  MachineSizeOpts.cpp
  MachineSSAContext.cpp
  MachineSSAUpdater.cpp
  MachineStripDebug.cpp
  MachineTraceMetrics.cpp
  MachineUniformityAnalysis.cpp
  MachineVerifier.cpp
  MIRFSDiscriminator.cpp
  MIRSampleProfile.cpp
  MIRYamlMapping.cpp
  MLRegAllocEvictAdvisor.cpp
  MLRegAllocPriorityAdvisor.cpp
  ModuloSchedule.cpp
  MultiHazardRecognizer.cpp
  PatchableFunction.cpp
  MBFIWrapper.cpp
  MIRPrinter.cpp
  MIRPrintingPass.cpp
  MacroFusion.cpp
  NonRelocatableStringpool.cpp
  OptimizePHIs.cpp
  PeepholeOptimizer.cpp
  PHIElimination.cpp
  PHIEliminationUtils.cpp
  PostRAHazardRecognizer.cpp
  PostRASchedulerList.cpp
  PreISelIntrinsicLowering.cpp
  ProcessImplicitDefs.cpp
  PrologEpilogInserter.cpp
  PseudoProbeInserter.cpp
  PseudoSourceValue.cpp
  RDFGraph.cpp
  RDFLiveness.cpp
  RDFRegisters.cpp
  ReachingDefAnalysis.cpp
  RegAllocBase.cpp
  RegAllocBasic.cpp
  RegAllocEvictionAdvisor.cpp
  RegAllocFast.cpp
  RegAllocGreedy.cpp
  RegAllocPBQP.cpp
  RegAllocPriorityAdvisor.cpp
  RegAllocScore.cpp
  RegisterClassInfo.cpp
  RegisterCoalescer.cpp
  RegisterPressure.cpp
  RegisterScavenging.cpp
  GCEmptyBasicBlocks.cpp
  RemoveRedundantDebugValues.cpp
  RenameIndependentSubregs.cpp
  MachineStableHash.cpp
  MIRVRegNamerUtils.cpp
  MIRNamerPass.cpp
  MIRCanonicalizerPass.cpp
  RegisterUsageInfo.cpp
  RegUsageInfoCollector.cpp
  RegUsageInfoPropagate.cpp
  RemoveLoadsIntoFakeUses.cpp
  ReplaceWithVeclib.cpp
  ResetMachineFunctionPass.cpp
  RegisterBank.cpp
  RegisterBankInfo.cpp
  SafeStack.cpp
  SafeStackLayout.cpp
  SanitizerBinaryMetadata.cpp
  ScheduleDAG.cpp
  ScheduleDAGInstrs.cpp
  ScheduleDAGPrinter.cpp
  ScoreboardHazardRecognizer.cpp
  SelectOptimize.cpp
  ShadowStackGCLowering.cpp
  ShrinkWrap.cpp
  SjLjEHPrepare.cpp
  SlotIndexes.cpp
  SpillPlacement.cpp
  SplitKit.cpp
  StackColoring.cpp
  StackFrameLayoutAnalysisPass.cpp
  StackMapLivenessAnalysis.cpp
  StackMaps.cpp
  StackProtector.cpp
  StackSlotColoring.cpp
  StaticDataSplitter.cpp
  StaticDataAnnotator.cpp
  SwiftErrorValueTracking.cpp
  SwitchLoweringUtils.cpp
  TailDuplication.cpp
  TailDuplicator.cpp
  TargetFrameLoweringImpl.cpp
  TargetInstrInfo.cpp
  TargetLoweringBase.cpp
  TargetLoweringObjectFileImpl.cpp
  TargetOptionsImpl.cpp
  TargetPassConfig.cpp
  TargetRegisterInfo.cpp
  TargetSchedule.cpp
  TargetSubtargetInfo.cpp
  TwoAddressInstructionPass.cpp
  TypePromotion.cpp
  UnreachableBlockElim.cpp
  ValueTypes.cpp
  VLIWMachineScheduler.cpp
  VirtRegMap.cpp
  WasmEHPrepare.cpp
  WindowScheduler.cpp
  WindowsSecureHotPatching.cpp
  WinEHPrepare.cpp
  XRayInstrumentation.cpp
  ${GeneratedMLSources}

  LiveDebugValues/LiveDebugValues.cpp
  LiveDebugValues/VarLocBasedImpl.cpp
  LiveDebugValues/InstrRefBasedImpl.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/CodeGen
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/CodeGen/PBQP

  LINK_LIBS ${LLVM_PTHREAD_LIB} ${MLLinkDeps}

  DEPENDS
  intrinsics_gen
  vt_gen
  ${MLDeps}

  LINK_COMPONENTS
  Analysis
  BitReader
  BitWriter
  CGData
  CodeGenTypes
  Core
  MC
  ObjCARC
  ProfileData
  Scalar
  Support
  Target
  TargetParser
  TransformUtils
  )

add_subdirectory(SelectionDAG)
add_subdirectory(AsmPrinter)
add_subdirectory(MIRParser)
add_subdirectory(GlobalISel)
