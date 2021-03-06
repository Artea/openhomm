// Copyright (c) 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// stackwalker_sparc.h: sparc-specific stackwalker.
//
// Provides stack frames given sparc register context and a memory region
// corresponding to an sparc stack.
//
// Author: Michael Shang


#ifndef PROCESSOR_STACKWALKER_SPARC_H__
#define PROCESSOR_STACKWALKER_SPARC_H__


#include "google_breakpad/common/breakpad_types.h"
#include "google_breakpad/common/minidump_format.h"
#include "google_breakpad/processor/stackwalker.h"

namespace google_breakpad {

class CodeModules;

class StackwalkerSPARC : public Stackwalker {
 public:
  // context is a sparc context object that gives access to sparc-specific
  // register state corresponding to the innermost called frame to be
  // included in the stack.  The other arguments are passed directly through
  // to the base Stackwalker constructor.
  StackwalkerSPARC(const SystemInfo *system_info,
                   const MDRawContextSPARC *context,
                   MemoryRegion *memory,
                   const CodeModules *modules,
                   SymbolSupplier *supplier,
                   SourceLineResolverInterface *resolver);

 private:
  // Implementation of Stackwalker, using x86 context (%ebp, %esp, %eip) and
  // stack conventions (saved %ebp at [%ebp], saved %eip at 4[%ebp], or
  // alternate conventions as guided by stack_frame_info_).
  // Implementation of Stackwalker, using ppc context (stack pointer in %r1,
  // saved program counter in %srr0) and stack conventions (saved stack
  // pointer at 0(%r1), return address at 8(0(%r1)).
  // Implementation of Stackwalker, using sparc context (%fp, %sp, %pc) and
  // stack conventions (saved %sp at)
  virtual StackFrame* GetContextFrame();
  virtual StackFrame* GetCallerFrame(
      const CallStack *stack,
      const vector< linked_ptr<StackFrameInfo> > &stack_frame_info);

  // Stores the CPU context corresponding to the innermost stack frame to
  // be returned by GetContextFrame.
  const MDRawContextSPARC *context_;
};


}  // namespace google_breakpad


#endif  // PROCESSOR_STACKWALKER_SPARC_H__
