# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

Aura 是一个基于 Unreal Engine 5.3 + Gameplay Ability System (GAS) 的俯视角 ARPG 学习项目。单 Runtime 模块 `Aura`（无 Editor 模块），插件：GameplayAbilities（内置）。代码注释为中文，保持此风格。

## 构建与运行

- 引擎为 launcher 安装版，位于 `D:\UE\UE_5.3`
- 打开项目：`D:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe "C:\Users\admin\Desktop\Aura\Aura.uproject"`
- 命令行编译：
  ```
  D:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat AuraEditor Win64 Development -project="C:\Users\admin\Desktop\Aura\Aura.uproject" -waitmutex
  ```
- 也可用 Visual Studio 打开 `Aura.sln` 或 Rider 打开 `Aura.uproject` 编译（两者配置目录均存在）
- **本项目没有任何自动化测试**。验证方式：编译成功 + 在编辑器中 PIE（默认地图为 `Content/Maps/StartupMap.umap`）

## 架构

### GAS 核心（Source/Aura/Public/AbilitySystem）

- `UAuraAbilitySystemComponent`：自定义 ASC，唯一职责是监听 `OnGameplayEffectAppliedDelegateToSelf` 并在 `EffectApplied` 中处理
- `UAuraAttributeSet`：Health / MaxHealth / Mana / MaxMana 四个属性，通过 `ATTRIBUTE_ACCESSORS` 宏生成 getter/setter。`PreAttributeChange` 中做 Clamp（注意：这里只做数值限制，不做网络复制，注释已说明原因）。属性带 `ReplicatedUsing` + `OnRep_*`。`FEffectProperties` 结构体用于在 `PostGameplayEffectExecute` 中提取效果的 Source/Target 上下文

### 角色层级（Source/Aura/Public/Character）

- `AAuraCharacterBase`（Abstract）：实现 `IAbilitySystemInterface`，持有 `AbilitySystemComponent` + `AttributeSet` 指针，有虚函数 `InitAbilityActorInfo()` 供子类覆写；另创建挂在 `WeaponHandSocket` 上的 `Weapon` 骨骼网格
- `AAuraCharacter`（玩家）：相机 + SpringArm；在 `PossessedBy` / `OnRep_PlayerState` 中调用 `InitAbilityActorInfo`——因为玩家的 ASC/AttributeSet 实际存放在 `AAuraPlayerState` 中（PlayerState 也实现了 `IAbilitySystemInterface`），角色的 `InitAbilityActorInfo` 负责将 PlayerState 的组件接到自身
- `AAuraEnemy`（敌人）：ASC/AttributeSet 在构造中创建；实现 `IEnemyInterface`（HighlightActor / UnHighlightActor）

### UI 的 MVC 模式（Source/Aura/Public/UI）

核心数据流：`AAuraHUD` → `UOverlayWidgetController` → `UAuraUserWidget`

1. `FWidgetControllerParams`（结构体）：打包 {PlayerController, PlayerState, ASC, AttributeSet} 四个指针
2. `AAuraHUD::InitOverlay`：创建 Overlay 控件和 `UOverlayWidgetController`，调用 `SetWidgetControllerParams` → `BindCallbacksToDependencies` → `BroadcastInitialValues`，最后 `Widget->SetWidgetController(Controller)`
3. `UOverlayWidgetController`：通过 `BlueprintAssignable` 动态多播委托（如 `FOnHealthChangedSignature`）向蓝图控件广播属性变化；监听 ASC 的 `GetGameplayAttributeValueChangeDelegate`
4. `UAuraUserWidget`：持有 `WidgetController` 属性，蓝图子类（WBP_Overlay 及 Health/Mana Globe）在此订阅委托

新增 UI 面板时沿用此模式：新建 WidgetController 子类 + 对应委托。

### 效果应用（Source/Aura/Public/Actor/AuraEffectActor.h）

`AAuraEffectActor` 是药水/水晶/区域效果的 C++ 基类，支持三类 GameplayEffect（Instant / Duration / Infinite），各自有 Application/Removal Policy 枚举控制叠加时机。蓝图资产位于 `Content/Blueprints/Actor/{Potion, Crystal, Area}`。

### GameplayTags

Tag 集中在 `Config/DefaultGameplayTags.ini`（当前为 `Attributes.Vital.*` 一族），另有 DataTable 方式（`Content/Blueprints/AbilitySystem/GameplayTags/DT_PrimaryAttributes`）。项目中三种建 Tag 方式（ini / 编辑器 Add new Gameplay Tag / DataTable）都用到过，修改 Tag 时注意保持一致。

### 输入

Enhanced Input：`IA_Move` + `IMC_AuraContext` 位于 `Content/Blueprints/Input`，在 `AAuraPlayerController` 中处理移动和 `CursorTrace`（鼠标 hover 敌人高亮，基于 `IEnemyInterface`）。

## 注意事项

- **`.uasset` / `.umap` 是二进制文件，Claude 无法编辑**——蓝图逻辑、GameplayEffect 数值、地图只能在 UE 编辑器中修改。C++ 改动需与用户确认蓝图侧是否联动
- `Config/DefaultEngine.ini` 中有 CoreRedirects（如 `AuraPlayerController.InputComponent` → `AuraContext`、`WidgetController` 结构体改名记录）。**重命名 C++ 类/属性/结构体时必须同步添加 CoreRedirect**，否则旧蓝图资产断链
- `Binaries/`、`Intermediate/`、`DerivedDataCache/`、`Saved/` 已 gitignore，无需关注
- 头文件顶部的 `// 3198` 是既有风格标记，新建文件保持即可
