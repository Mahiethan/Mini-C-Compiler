; ModuleID = 'testThree.c'
source_filename = "testThree.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @firstFunc(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %6 = load i32, ptr %4, align 4
  %7 = icmp ne i32 %6, 0
  %8 = xor i1 %7, true
  %9 = zext i1 %8 to i32
  %10 = sub nsw i32 0, %9
  %11 = mul nsw i32 4, %10
  %12 = add nsw i32 3, %11
  store i32 %12, ptr %5, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @secondFunc(float noundef %0) #0 {
  %2 = alloca float, align 4
  %3 = alloca float, align 4
  store float %0, ptr %2, align 4
  %4 = load float, ptr %3, align 4
  %5 = fpext float %4 to double
  %6 = fcmp oeq double %5, 4.000000e+00
  br i1 %6, label %7, label %16

7:                                                ; preds = %1
  %8 = load float, ptr %2, align 4
  %9 = fcmp une float %8, 0.000000e+00
  %10 = xor i1 %9, true
  %11 = zext i1 %10 to i32
  %12 = sub nsw i32 0, %11
  %13 = mul nsw i32 4, %12
  %14 = sub nsw i32 3, %13
  %15 = sitofp i32 %14 to float
  store float %15, ptr %3, align 4
  br label %17

16:                                               ; preds = %1
  store float 7.000000e+00, ptr %3, align 4
  br label %17

17:                                               ; preds = %16, %7
  br label %18

18:                                               ; preds = %21, %17
  %19 = load float, ptr %3, align 4
  %20 = fcmp oeq float %19, 7.000000e+00
  br i1 %20, label %21, label %22

21:                                               ; preds = %18
  store float 7.000000e+00, ptr %3, align 4
  br label %18, !llvm.loop !4

22:                                               ; preds = %18
  ret i32 1
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 15.0.7 (Red Hat 15.0.7-1.module+el8.8.0+1144+0a4e73bd)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
