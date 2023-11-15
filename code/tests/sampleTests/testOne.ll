; ModuleID = 'testOne.c'
source_filename = "testOne.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

@a = dso_local global i32 0, align 4
@b = dso_local global float 0.000000e+00, align 4
@c = dso_local global i8 0, align 1
@o = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local float @asd() #0 {
entry:
  %a = alloca float, align 4
  %0 = load float, ptr %a, align 4
  ret float %0
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i1 @test(i32 noundef %a, float noundef %b, i1 noundef zeroext %c) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca float, align 4
  %c.addr = alloca i8, align 1
  %test = alloca float, align 4
  %aaa = alloca i8, align 1
  store i32 %a, ptr %a.addr, align 4
  store float %b, ptr %b.addr, align 4
  %frombool = zext i1 %c to i8
  store i8 %frombool, ptr %c.addr, align 1
  %0 = load i8, ptr %aaa, align 1
  %tobool = trunc i8 %0 to i1
  ret i1 %tobool
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @tester(i32 noundef %v) #0 {
entry:
  %v.addr = alloca i32, align 4
  store i32 %v, ptr %v.addr, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @ret(float noundef %h) #0 {
entry:
  %h.addr = alloca float, align 4
  store float %h, ptr %h.addr, align 4
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %or = alloca i8, align 1
  %or1 = alloca i32, align 4
  %or2 = alloca float, align 4
  %and = alloca i8, align 1
  %eq = alloca float, align 4
  %neq = alloca i8, align 1
  %le = alloca i8, align 1
  %lt = alloca i8, align 1
  %ge = alloca i8, align 1
  %gt = alloca i8, align 1
  %plus = alloca float, align 4
  %minus = alloca float, align 4
  %mod = alloca float, align 4
  %div = alloca float, align 4
  %mult = alloca float, align 4
  %unary = alloca float, align 4
  %not = alloca float, align 4
  %combo = alloca float, align 4
  store i32 0, ptr %retval, align 4
  br i1 true, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %entry
  %0 = phi i1 [ true, %entry ], [ false, %lor.rhs ]
  %frombool = zext i1 %0 to i8
  store i8 %frombool, ptr %or, align 1
  br i1 true, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %lor.end
  br label %land.end

land.end:                                         ; preds = %land.rhs, %lor.end
  %1 = phi i1 [ false, %lor.end ], [ false, %land.rhs ]
  %frombool1 = zext i1 %1 to i8
  store i8 %frombool1, ptr %and, align 1
  store float 1.000000e+00, ptr %eq, align 4
  store i8 1, ptr %neq, align 1
  store i8 0, ptr %le, align 1
  store i8 0, ptr %lt, align 1
  store i8 1, ptr %ge, align 1
  store i8 1, ptr %gt, align 1
  store float 1.000000e+00, ptr %plus, align 4
  store float 1.000000e+00, ptr %minus, align 4
  store float 0.000000e+00, ptr %mult, align 4
  store float 0x40590AE140000000, ptr %div, align 4
  store float 0.000000e+00, ptr %div, align 4
  store float 0.000000e+00, ptr %mod, align 4
  store float -1.000000e+02, ptr %unary, align 4
  store float 0.000000e+00, ptr %not, align 4
  store float 0xC11927C240000000, ptr %combo, align 4
  store float 1.000000e+00, ptr %combo, align 4
  %call = call i32 @ret(float noundef 1.000000e+00)
  ret i32 %call
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 15.0.7 (Red Hat 15.0.7-1.module+el8.8.0+1144+0a4e73bd)"}
