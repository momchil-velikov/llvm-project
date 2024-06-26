; RUN: opt %loadNPMPolly '-passes=print<polly-function-scops>' -disable-output < %s 2>&1 | FileCheck %s
;
; Unsigned wrap-around check.
;
; for (int i = -1; i < 65 ; i ++ )
;   if ( (unsigned)i < 64 )
;     A[i] = 42;


define void @func(ptr noalias nonnull %A) {
entry:
  br label %for

  for:
  %j = phi i32 [-1, %entry], [%j.inc, %inc]
  %j.cmp = icmp slt i32 %j, 65
  br i1 %j.cmp, label %body, label %exit

  body:
  %inbounds = icmp ult i32 %j, 64
  br i1 %inbounds, label %ifinbounds, label %ifoutbounds

  ifinbounds:
  %A_idx = getelementptr inbounds double, ptr %A, i32 %j
  store double 42.0, ptr %A_idx
  br label %inc

  ifoutbounds:
  br label %inc

  inc:
  %j.inc = add nuw nsw i32 %j, 1
  br label %for

  exit:
  br label %return

  return:
  ret void
}


; CHECK: Region: %for---%return
; CHECK:            Domain :=
; CHECK-NEXT:                            { Stmt_ifinbounds[i0] : 0 < i0 <= 64 };

