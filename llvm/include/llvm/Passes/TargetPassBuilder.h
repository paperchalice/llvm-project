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
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace llvm {

class PassBuilder;
class TargetMachine;

class TargetPassBuilder {
public:
  TargetPassBuilder(PassBuilder &PB);

  virtual ~TargetPassBuilder() = default;

  virtual ModulePassManager buildPipeline();

private:
  template <typename InternalPassT> struct AdaptorWrapper {
    InternalPassT P;
  };
  template <> struct AdaptorWrapper<void> {};

  template <typename PassManagerT, typename InternalPassT = void>
  class PassManagerWrapper {
  public:
    bool isEmpty() const { return Passes.empty(); }

    template <typename PassT> void addPass(PassT &&P) {
      Passes.emplace_back(PassT::name(),
                          PassManagerT().addPass(std::forward<PassT>(P)));
    }

    void addPass(PassManagerWrapper &&PM) {
      Passes.insert(Passes.end(), std::make_move_iterator(PM.Passes.begin()),
                    std::make_move_iterator(PM.Passes.end()));
    }

    template <> void addPass(AdaptorWrapper<InternalPassT> &&Adaptor) {
      Passes.push_back(std::move(Adaptor.P));
    }

    template <> void addPass(AdaptorWrapper<void> &&) = delete;

    void addPass(llvm::ModulePassManager &&) = delete;
    void addPass(llvm::FunctionPassManager &&) = delete;
    void addPass(llvm::LoopPassManager &&) = delete;
    void addPass(llvm::MachineFunctionPassManager &&) = delete;

  private:
    std::vector<std::pair<
        StringRef,
        std::variant<llvm::ModulePassManager, llvm::FunctionPassManager,
                     llvm::LoopPassManager, llvm::MachineFunctionPassManager>>>
        Passes;
  };

  template <typename PassT, typename NestedPassManagerT>
  AdaptorWrapper<NestedPassManagerT> createPassAdaptor(PassT &&P) {
    AdaptorWrapper<NestedPassManagerT> Adaptor;
    Adaptor.P.add_pass(std::forward<PassT>(P));
    return Adaptor;
  }

protected:
  // Shadow real pass managers intentionally.
  using MachineFunctionPassManager =
      PassManagerWrapper<llvm::MachineFunctionPassManager>;
  using LoopPassManager = PassManagerWrapper<llvm::LoopPassManager>;
  using FunctionPassManager =
      PassManagerWrapper<llvm::FunctionPassManager, LoopPassManager>;
  using ModulePassManager =
      PassManagerWrapper<llvm::ModulePassManager, FunctionPassManager>;

private:
  template <typename NestedPassManagerT, typename PassT>
  AdaptorWrapper<NestedPassManagerT> createPassAdaptor(PassT &&P) {
    AdaptorWrapper<NestedPassManagerT> Adaptor;
    Adaptor.P.add_pass(std::forward<PassT>(P));
    return Adaptor;
  }

protected:
  template <typename FunctionPassT>
  AdaptorWrapper<FunctionPassManager>
  createModuleToFunctionPassAdaptor(FunctionPassT &&P) {
    return createPassAdaptor<FunctionPassManager>(
        std::forward<FunctionPassT>(P));
  }

  template <typename LoopPassT>
  AdaptorWrapper<LoopPassManager>
  createFunctionToLoopPassAdaptor(LoopPassT &&P) {
    return createPassAdaptor<LoopPassManager>(std::forward<LoopPassT>(P));
  }

  template <typename MachineFunctionPassT>
  AdaptorWrapper<MachineFunctionPassManager>
  createFunctionToMachineFunctionPassAdaptor(MachineFunctionPassT &&P) {
    return createPassAdaptor<MachineFunctionPassManager>(
        std::forward<MachineFunctionPassManager>(P));
  }

protected:
  PassBuilder &PB;
  CGPassBuilderOption CGPBO = getCGPassBuilderOption();

  void buildCodeGenIRPipeline();

  void buildExceptionHandlingPipeline();

  void buildISelPreparePipeline();

  virtual void addISelIRPasses();

  void addCoreISelPasses();

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
