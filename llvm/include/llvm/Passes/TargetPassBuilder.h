//===- Parsing, selection, and construction of pass pipelines --*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// Interfaces for registering analysis passes, producing common pass manager
/// configurations, and parsing of pass pipelines.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_PASSES_TARGETPASSBUILDER_H
#define LLVM_PASSES_TARGETPASSBUILDER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/CodeGen/MachinePassManager.h"
#include "llvm/Target/CGPassBuilderOption.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include <utility>

namespace llvm {

class PassBuilder;
class TargetMachine;

class TargetPassBuilder {
public:
  TargetPassBuilder(PassBuilder &PB);

  virtual ~TargetPassBuilder() = default;

  virtual ModulePassManager buildPipeline();

protected:
  PassBuilder &PB;
  CGPassBuilderOption CGPBO = getCGPassBuilderOption();

  void buildCodeGenIRPipeline();

  void buildExceptionHandlingPipeline();

  void buildISelPreparePipeline();

  virtual void addISelIRPasses();

  void addCoreISelPasses();

  /// \defgroup AddPass Helper method to add passes
  /// @{
  template <typename... PassTs>
  TargetPassBuilder &addModulePass(PassTs &&...Passes) {
    static_assert(!(isPassManager<PassTs> || ...), "");
    static_assert((isModulePass<PassTs> && ...),
                  "All passes should be module pass!");
    (addPassInteral(FinalMPM, std::forward<PassTs>(Passes)), ...);
    return *this;
  }

  template <typename... PassTs>
  TargetPassBuilder &addFunctionPass(PassTs &&...Passes) {
    static_assert(!(isPassManager<PassTs> || ...), "");
    static_assert((isFunctionPass<PassTs> && ...),
                  "All passes should be function pass!");
    (addPassInternal(FinalFPM, std::forward<PassTs>(Passes)), ...);
    return *this;
  }

  template <typename... PassTs>
  TargetPassBuilder &addLoopPass(PassTs &&...Passes) {
    static_assert(!(isPassManager<PassTs> || ...), "");
    static_assert((isLoopPass<PassTs> && ...),
                  "All passes should be loop pass!");
    (addPassInternal(FinalLPM, std::forward<PassTs>(Passes)), ...);
    return *this;
  }

  template <typename... PassTs>
  TargetPassBuilder &addMachineFunctionPass(PassTs &&...Passes) {
    static_assert(!(isPassManager<PassTs> || ...), "");
    static_assert((isMachineFunctionPass<PassTs> && ...),
                  "All passes should be machine function pass!");
    (addPassInternal(FinalMFPM, std::forward<PassTs>(Passes)), ...);
    return *this;
  }
  /// @}

  template <typename PassT> void injectBefore(std::function<void()> F) {
    BeforeAddingCallbacks.push_back([Accessed = false](StringRef Name) mutable {
      if (Accessed)
        return true;
      Accessed = true;
      if (PassT::name() != Name)
        return true;
      F();
      return true;
    });
  }

  template <typename PassT> void injectAfter(std::function<void()> F) {
    AfterAddingCallbacks.push_back([Accessed = false](StringRef Name) mutable {
      if (Accessed)
        return;
      Accessed = true;
      if (PassT::name() != Name)
        return;
      F();
    });
  }

  template <typename... PassTs> void disablePass() {
    (DisabedPasses.insert(PassTs::name()), ...);
  }

private:
  void buildCoreCodeGenPipeline();

private:
  virtual void anchor();

  StringSet<> DisabedPasses;

  template <typename PassT, typename IRUnitT,
            typename AnalysisManagerT = AnalysisManager<IRUnitT>,
            typename = void, typename... ExtraArgTs>
  struct IsIRUnitPass : std::false_type {};
  template <typename PassT, typename IRUnitT, typename AnalysisManagerT,
            typename... ExtraArgTs>
  struct IsIRUnitPass<
      IRUnitT, AnalysisManagerT, PassT,
      std::void_t<decltype(std::declval<PassT>().run(
          std::declval<IRUnitT>(), std::declval<AnalysisManagerT>(),
          std::declval<ExtraArgTs>()...))>> : std::true_type {};

  template <typename PassT>
  static constexpr bool isPassManager =
      std::is_same_v<PassT, ModulePassManager> ||
      std::is_same_v<PassT, FunctionPassManager> ||
      std::is_same_v<PassT, LoopPassManager> ||
      std::is_same_v<PassT, MachineFunctionPassManager>;

  template <typename PassT>
  static constexpr bool isPassAdaptor =
      std::is_same_v<PassT, ModuleToFunctionPassAdaptor> ||
      std::is_same_v<PassT, FunctionToLoopPassAdaptor> ||
      std::is_same_v<PassT, FunctionToMachineFunctionPassAdaptor>;

  template <typename PassT>
  static constexpr bool isModulePass = IsIRUnitPass<PassT, Module>::value;

  template <typename PassT>
  static constexpr bool isFunctionPass = IsIRUnitPass<PassT, Function>::value;

  template <typename PassT>
  static constexpr bool isLoopPass =
      IsIRUnitPass<PassT, Loop, LoopAnalysisManager,
                   LoopStandardAnalysisResults &, LPMUpdater &>::value;

  template <typename PassT>
  static constexpr bool isMachineFunctionPass =
      IsIRUnitPass<PassT, MachineFunction>::value;

  ModulePassManager FinalMPM;
  FunctionPassManager FinalFPM;
  LoopPassManager FinalLPM;
  MachineFunctionPassManager FinalMFPM;

  ModulePassManager getFinalPM() {
    if (!FinalMFPM.isEmpty()) {
      FinalFPM.addPass(
          createFunctionToMachineFunctionPassAdaptor(std::move(FinalMFPM)));
    }
    if (!FinalFPM.isEmpty()) {
      FinalMPM.addPass(createModuleToFunctionPassAdaptor(std::move(FinalFPM)));
    }
    return std::move(FinalMPM);
  }

  template <typename PassManagerT, typename PassT>
  void addPassInternal(PassManagerT &PM, PassT &&P) {
    if (invokeBeforeAddingCallbacks(PassT::name()))
      return;

    if constexpr (isModulePass<PassT>) {
      if (!FinalLPM.isEmpty()) {
        FinalFPM.addPass(
            createFunctionToLoopPassAdaptor(std::move(FinalLPM),
                                            /*UseMemorySSA=*/true));
        FinalLPM = LoopPassManager();
      }
      if (!FinalMFPM.isEmpty()) {
        FinalFPM.addPass(
            createFunctionToMachineFunctionPassAdaptor(std::move(FinalMFPM)));
        FinalMFPM = MachineFunctionPassManager();
      }
      if (!FinalFPM.isEmpty()) {
        FinalMPM.addPass(
            createModuleToFunctionPassAdaptor(std::move(FinalFPM)));
        FinalFPM = FunctionPassManager();
      }
    }
    if constexpr (isFunctionPass<PassT>) {
      if (!FinalLPM.isEmpty()) {
        FinalFPM.addPass(
            createFunctionToLoopPassAdaptor(std::move(FinalLPM),
                                            /*UseMemorySSA=*/true));
        FinalLPM = LoopPassManager();
      }
      if (!FinalMFPM.isEmpty()) {
        FinalFPM.addPass(
            createFunctionToMachineFunctionPassAdaptor(std::move(FinalMFPM)));
        FinalMFPM = MachineFunctionPassManager();
      }
    }

    PM.addPass(std::forward<PassT>(P));
    invokeAfterAddingCallbacks(PassT::name());
  }

  SmallVector<std::function<bool(StringRef)>, 3> BeforeAddingCallbacks;
  SmallVector<std::function<void(StringRef)>, 2> AfterAddingCallbacks;

  bool invokeBeforeAddingCallbacks(StringRef Name) const {
    bool ShouldAdd = true;
    for (auto &C : BeforeAddingCallbacks)
      ShouldAdd &= C(Name);
    return ShouldAdd;
  }

  void invokeAfterAddingCallbacks(StringRef Name) {
    for (auto &C : AfterAddingCallbacks)
      C(Name);
  }
};

} // namespace llvm

#endif
