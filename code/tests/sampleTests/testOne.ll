; ModuleID = 'testOne.c'
source_filename = "testOne.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

@o = dso_local global i32 0, align 4
@a = dso_local global i32 0, align 4
@b = dso_local global float 0.000000e+00, align 4
@c = dso_local global i8 0, align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local float @asd() #0 {
entry:
  %a = alloca float, align 4
  %0 = load float, ptr %a, align 4
  ret float %0
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i1 @test(i32 noundef %a, i32 noundef %b, i1 noundef zeroext %c) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %c.addr = alloca i8, align 1
  %test = alloca float, align 4
  %aaa = alloca i8, align 1
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %frombool = zext i1 %c to i8
  store i8 %frombool, ptr %c.addr, align 1
  store float 5.000000e+00, ptr %test, align 4
  store i8 0, ptr %aaa, align 1
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %add = add nsw i32 %0, %1
  %2 = load i8, ptr %c.addr, align 1
  %tobool = trunc i8 %2 to i1
  %conv = zext i1 %tobool to i32
  %add1 = add nsw i32 %add, %conv
  %conv2 = sitofp i32 %add1 to float
  store float %conv2, ptr %test, align 4
  %3 = load i8, ptr %aaa, align 1
  %tobool3 = trunc i8 %3 to i1
  ret i1 %tobool3
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @addition(i32 noundef %n, i32 noundef %m) #0 {
entry:
  %n.addr = alloca i32, align 4
  %m.addr = alloca i32, align 4
  %result = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  store i32 %m, ptr %m.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = load i32, ptr %m.addr, align 4
  %add = add nsw i32 %0, %1
  store i32 %add, ptr %result, align 4
  %2 = load i32, ptr %result, align 4
  ret i32 %2
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
  %combo = alloca float, align 4
  %ty = alloca i32, align 4
  %flo = alloca i32, align 4
  %A = alloca i32, align 4
  %hg_o = alloca i32, align 4
  %hg = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = load i32, ptr @o, align 4
  store i32 %0, ptr %ty, align 4
  %1 = load i32, ptr @a, align 4
  %cmp = icmp eq i32 %1, 10
  br i1 %cmp, label %if.then, label %if.else8

if.then:                                          ; preds = %entry
  br label %while.cond

while.cond:                                       ; preds = %if.end, %if.then
  %2 = load i32, ptr %ty, align 4
  %cmp1 = icmp eq i32 %2, 10
  br i1 %cmp1, label %while.body, label %while.end7

while.body:                                       ; preds = %while.cond
  store i32 20, ptr @o, align 4
  %3 = load i32, ptr %ty, align 4
  %cmp2 = icmp ne i32 %3, 19
  br i1 %cmp2, label %if.then3, label %if.else

if.then3:                                         ; preds = %while.body
  br label %while.cond4

while.cond4:                                      ; preds = %while.body6, %if.then3
  %4 = load i32, ptr @a, align 4
  %cmp5 = icmp eq i32 %4, 10
  br i1 %cmp5, label %while.body6, label %while.end

while.body6:                                      ; preds = %while.cond4
  %5 = load i32, ptr @a, align 4
  %sub = sub nsw i32 %5, 1
  store i32 %sub, ptr %hg, align 4
  br label %while.cond4, !llvm.loop !4

while.end:                                        ; preds = %while.cond4
  store i32 0, ptr %hg, align 4
  br label %if.end

if.else:                                          ; preds = %while.body
  store i32 10, ptr %ty, align 4
  store i32 0, ptr %hg_o, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %while.end
  br label %while.cond, !llvm.loop !6

while.end7:                                       ; preds = %while.cond
  store i32 10, ptr @o, align 4
  br label %if.end9

if.else8:                                         ; preds = %entry
  store i32 190, ptr %flo, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.else8, %while.end7
  %6 = load i32, ptr @o, align 4
  %sub10 = sub nsw i32 %6, -9
  %conv = sitofp i32 %sub10 to float
  store float %conv, ptr %combo, align 4
  store float 5.000000e+00, ptr %combo, align 4
  ret i32 0
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
!6 = distinct !{!6, !5}
