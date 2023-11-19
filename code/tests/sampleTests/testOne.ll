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
  %a = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = load i32, ptr @o, align 4
  store i32 %0, ptr %ty, align 4
  %1 = load i32, ptr @a, align 4
  %cmp = icmp eq i32 %1, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end10

if.else:                                          ; preds = %entry
  store i32 1, ptr %a, align 4
  %2 = load i32, ptr %a, align 4
  %cmp1 = icmp eq i32 %2, 1
  br i1 %cmp1, label %if.then2, label %if.else5

if.then2:                                         ; preds = %if.else
  store i32 0, ptr %a, align 4
  %3 = load i32, ptr %a, align 4
  %cmp3 = icmp eq i32 %3, 3
  br i1 %cmp3, label %if.then4, label %if.end

if.then4:                                         ; preds = %if.then2
  br label %if.end

if.end:                                           ; preds = %if.then4, %if.then2
  store i32 0, ptr %retval, align 4
  br label %return

if.else5:                                         ; preds = %if.else
  store i32 10, ptr @o, align 4
  br label %if.end6

if.end6:                                          ; preds = %if.else5
  %4 = load i32, ptr %a, align 4
  %cmp7 = icmp eq i32 %4, 10000
  br i1 %cmp7, label %if.then8, label %if.end9

if.then8:                                         ; preds = %if.end6
  br label %if.end9

if.end9:                                          ; preds = %if.then8, %if.end6
  store i32 999, ptr %a, align 4
  br label %if.end10

if.end10:                                         ; preds = %if.end9, %if.then
  %5 = load i32, ptr @o, align 4
  %sub = sub nsw i32 %5, -9
  %conv = sitofp i32 %sub to float
  store float %conv, ptr %combo, align 4
  store float 5.000000e+00, ptr %combo, align 4
  store i32 0, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.end10, %if.end
  %6 = load i32, ptr %retval, align 4
  ret i32 %6
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 15.0.7 (Red Hat 15.0.7-1.module+el8.8.0+1144+0a4e73bd)"}
