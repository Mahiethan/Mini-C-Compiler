; ModuleID = 'testOne.c'
source_filename = "testOne.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

@a = dso_local global i32 0, align 4
@o = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local float @asd() #0 {
  %1 = alloca float, align 4
  %2 = load float, ptr %1, align 4
  ret float %2
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i1 @test(i32 noundef %0, float noundef %1, i1 noundef zeroext %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca float, align 4
  %6 = alloca i8, align 1
  %7 = alloca float, align 4
  %8 = alloca i8, align 1
  store i32 %0, ptr %4, align 4
  store float %1, ptr %5, align 4
  %9 = zext i1 %2 to i8
  store i8 %9, ptr %6, align 1
  %10 = load i8, ptr %8, align 1
  %11 = trunc i8 %10 to i1
  ret i1 %11
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @tester(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @ret(float noundef %0) #0 {
  %2 = alloca float, align 4
  store float %0, ptr %2, align 4
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %4 = load i32, ptr %2, align 4
  call void @tester(i32 noundef %4)
  %5 = load i32, ptr %3, align 4
  call void @tester(i32 noundef %5)
  %6 = call i32 @ret(float noundef 9.000000e+01)
  ret i32 %6
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 15.0.7 (Red Hat 15.0.7-1.module+el8.8.0+1144+0a4e73bd)"}
