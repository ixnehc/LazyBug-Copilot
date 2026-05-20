#pragma once

#include "GObj.h"



#define GObjUID_EmitterParam 1
#define GObjUID_ParArg 2
#define GObjUID_PNull 3

#define GObjUID_PGravSpn 4
#define GObjUID_PAccUpd 5
#define GObjUID_PColSpn 6
#define GObjUID_PColSpnS 7
#define GObjUID_PAlphaUpd 8
#define GObjUID_PAlphaUpdA 9
#define GObjUID_PAlphaFO 10
#define GObjUID_PFrmUpdA 11
#define GObjUID_PLifeSpn 12
#define GObjUID_PLifeSpnS 13
#define GObjUID_PPosSpnBox 14
#define GObjUID_PPosSpnPnt 15
#define GObjUID_PPosUpd 16
#define GObjUID_PRotSpn 17
#define GObjUID_PRotUpd 18
#define GObjUID_PRotVelSpn 19
#define GObjUID_PRotVelSpnS 20
#define GObjUID_PRotVelUpdA 21
#define GObjUID_PSzSpn 22
#define GObjUID_PSzSpnS 23
#define GObjUID_PSzUpd 24
#define GObjUID_PSzUpdA 25
#define GObjUID_PSzFO 26
#define GObjUID_PVelSpnRange 27
#define GObjUID_PVelSpnAlign 28
#define GObjUID_PVelUpd 29
#define GObjUID_PVelUpd_A 30
#define GObjUID_PathArg 31
#define GObjUID_AnimUVArg 32
#define GObjUID_PRotSpnPnt 33
#define GObjUID_EmitterParArg 34
#define GObjUID_SpriteParArg 35
#define GObjUID_RippleParArg 36
#define GObjUID_RippleParam  37
#define GObjUID_PPosSpnRadius 38
#define GObjUID_PPosSpnTube 39
#define GObjUID_PRotVelUpd 40
#define GObjUID_PRotSpnAlign 41
#define GObjUID_BubbleModeParam_Default 42
#define GObjUID_DLMP_PointDur 43
#define GObjUID_DLMP_Point 44
#define GObjUID_DLMP_Null 45
#define GObjUID_TunerParamCurve 46
#define GObjUID_PColorUpd 47
#define GObjUID_BubbleModeParam_Infinite 48




#define GObjUID_CBgp_CheckItem 49
#define GObjUID_CBgp_StartRelay			  54
#define GObjUID_CBgp_Relay					  55
#define GObjUID_CBgp_Repeater_Obsolete				  57
#define GObjUID_CBgp_Parallel				   58
#define GObjUID_CBgp_Random				  59
#define GObjUID_CBgp_Attempt_Obsolete				  75
#define GObjUID_CBgp_Xto1					  77
#define GObjUID_CBgp_Fail						  79
#define GObjUID_CBgp_Rate					  81
#define GObjUID_CBgp_Delay_Obsolete					   82
#define GObjUID_CBgp_State					  83
#define GObjUID_CBgp_SwitchState		  84
#define GObjUID_CBgp_ActivateStates	  85
#define GObjUID_CBgp_TerminateState	  86
#define GObjUID_CBgp_Include				  87
#define GObjUID_CBgp_StubIn					  88
#define GObjUID_CBgp_StubOut				  89
#define GObjUID_CBgp_Graph						96
#define GObjUID_CBgp_Counter				   97
#define GObjUID_CBgp_Timer					  98
#define GObjUID_CBgp_Register				  99
#define GObjUID_CBgp_Var                       100
#define GObjUID_CBgp_Consts                 101
#define GObjUID_CBgp_MonitorMasterLeave 102
#define GObjUID_CBgp_CheckRtnuCmd  103
#define GObjUID_CBgp_MonitorService 104
#define GObjUID_CBgp_AddService 105
#define GObjUID_CBgp_ClaimServiceWithSkill 106
#define GObjUID_CBgp_ServiceQuota 107
#define GObjUID_CBgp_MonitorCoSkill 108
#define GObjUID_CBgp_FindNearPos 109
#define GObjUID_CBgp_DetectSpecifiedUnit 110
#define GObjUID_CBgp_DetectSignal 111
#define GObjUID_CBgp_DetectResidable 112
#define GObjUID_CBgp_CheckMasterDist 113
#define GObjUID_CBgp_DetectSpecifiedAgent 114
#define GObjUID_CBgp_CheckVar 115
#define GObjUID_CBgp_CompareBool_Obsolete 116
#define GObjUID_CBgp_CheckTimer_Obsolete 117
#define GObjUID_CBgp_CheckResiding 118
#define GObjUID_CBgp_GetLevelObjPos 119
#define GObjUID_CBgp_CheckPlayerRes 120
#define GObjUID_CBgp_Teleport 121
#define GObjUID_CBgp_SyncSpeed 122
#define GObjUID_CBgp_CheckHP 123
#define GObjUID_CBgp_Stroll 124
#define GObjUID_CBgp_CheckEquip 125
#define GObjUID_CBgp_CheckBuff 126
#define GObjUID_CBgp_CheckCounter 127
#define GObjUID_CBgp_RaiseUp 128
#define GObjUID_CBgp_Reside 129
#define GObjUID_CBgp_CheckDist 130
#define GObjUID_CBgp_MoveAlong 131
#define GObjUID_CBgp_SwitchRetinue 132
#define GObjUID_CBgp_SpawnUnit 133
#define GObjUID_CBgp_ResetTimer_Obsolete 134
#define GObjUID_CBgp_LockPlayer 135
#define GObjUID_CBgp_KnockDown 136
#define GObjUID_CBgp_FlyUp 137
#define GObjUID_CBgp_Siege 138
#define GObjUID_CBgp_FlyThrust 139
#define GObjUID_CBgp_ModCounter 140
#define GObjUID_CBgp_ModVar_Obsolete 141
#define GObjUID_CBgp_FlyDown 144
#define GObjUID_CBgp_Follow_Obsolete 145
#define GObjUID_CBgp_SetFaction 146
#define GObjUID_CBgp_Flee 147
#define GObjUID_CBgp_Equip 148
#define GObjUID_CBgp_SendEvent 149
#define GObjUID_CBgp_Die 150
#define GObjUID_CBgp_Attack 151
#define GObjUID_CBgp_AttackNoTarget 152
#define GObjUID_CBgp_AttackTarget 153
#define GObjUID_CBgp_AttackHoldPos 154
#define GObjUID_CBgp_CancelSkill 155
#define GObjUID_CBgp_TalkAddChoice 156
#define GObjUID_CBgp_TalkSentence 157
#define GObjUID_CBgp_TalkSpeak 158
#define GObjUID_CBgp_TalkDialog 159
#define GObjUID_CBgp_TalkPopup 160
#define GObjUID_CBgp_BreakTalk 161
#define GObjUID_CBgp_WaitTalk 162
#define GObjUID_CBgp_RecordTalkPlayer 163
#define GObjUID_CBgp_DestroyLo 164
#define GObjUID_CBgp_Invisible 165
#define GObjUID_CBgp_Revive 166
#define GObjUID_CBgp_Accompany_Obsolete 167
#define GObjUID_CBgp_CheckKilling 168
#define GObjUID_CBgp_FlyingHover 169
#define GObjUID_CBgp_Repeater 170
#define GObjUID_CBgp_CheckAIScenario 171
#define GObjUID_CBgp_GetLevelObjID 172
#define GObjUID_CBgpTroop_MoveTo 173
#define GObjUID_CBgpMoveTo 174
#define GObjUID_CBgp_Loop 175
#define GObjUID_CBgp_SetCollide 176
#define GObjUID_CBgp_CompareInt_Obsolete 177
#define GObjUID_CBgpGA_ModRes 178
#define GObjUID_CBgp_ModVar 179
#define GObjUID_CBgp_CompareNumber 180
#define GObjUID_CBgp_GetTalkDialogParam 181
#define GObjUID_CBgp_GetDay 182
#define GObjUID_CBgp_Sequence 183
#define GObjUID_CBgp_Succeed 184
#define GObjUID_CBgp_Selector 185
#define GObjUID_CBgpGA_Revive 186
#define GObjUID_CBgpAGA_Call 187
#define GObjUID_CBgpTroop_Build 188
#define GObjUID_CBgp_AreaOp 189
#define GObjUID_CBgp_Interrupt 190
#define GObjUID_CBgpCmd_MoveTo 191
#define GObjUID_CBgpTroop_MoveAlong 192
#define GObjUID_CBgpTroop_Detect 193
#define GObjUID_CBgpTroop_Combat 194
#define GObjUID_CBgp_Strategy 195
#define GObjUID_CBgpTroop_CheckDist 196
#define GObjUID_CBgp_Proxy 197
#define GObjUID_CBgpAGA_CheckState 198
#define GObjUID_CBgp_Decision 199
#define GObjUID_CBgp_Func 200
#define GObjUID_CBgp_Call 201
#define GObjUID_CBgp_Vars 202
#define GObjUID_CBgp_Troops 203
#define GObjUID_CBgpAI_CheckCmd 204
#define GObjUID_CBgpTroop_CancelCmd 205
#define GObjUID_CUpgradeFire_Init 206
#define GObjUID_CUpgradeFire_IncDmg 207
#define GObjUID_CUpgradeFire_DecCost 208
#define GObjUID_CLevelAbility_Fire 209
#define GObjUID_RollAwardsResult 210
#define GObjUID_CLevelAbility_Unarmed 211
#define GObjUID_CLevelAbility_UtumTide 212
#define GObjUID_CBgp_UtumThrust 213
#define GObjUID_CLevelAbility_FlashSwing 214
#define GObjUID_CLevelAbility_DeathCall 215
#define GObjUID_CBgp_MakeBuff 216
#define GObjUID_CBgp_CheckEventSrc 217
#define GObjUID_CBgp_RemoveBuff 218
#define GObjUID_LevelDialogInfo 219
#define GObjUID_CBgp_ResetTimer 220
#define GObjUID_CBgp_CheckTimer 221
#define GObjUID_CBgp_MonitorStuck 222
#define GObjUID_CBgp_JumpAttack 223
#define GObjUID_CBgp_CheckSkill 224
#define GObjUID_CBgp_CheckJumpAttack 225
#define GObjUID_LevelAttackAddOn_Deal 226
#define GObjUID_DealEntry 227
#define GObjUID_CBgp_CheckObstacle 228
#define GObjUID_BMO_PathSafeSrc_obsolete 229
#define GObjUID_CBgpThreat_CheckCast 230
#define GObjUID_CBgpThreat_Attack 231
#define GObjUID_CBgpThreat_Approach 232
#define GObjUID_CBgpThreat_Cast 233
#define GObjUID_CBgpThreat_CheckRange 234
#define GObjUID_CBgp_Follow 235
#define GObjUID_CBgp_StopMove 236
#define GObjUID_CBgp_CheckMoveMethod 237
#define GObjUID_PFrmUpd 238
#define GObjUID_CBgp_FindNearPos_Special 239
#define GObjUID_CBgp_RandomizeVar 240
#define GObjUID_CBgpThreat_CheckExist 241
#define GObjUID_CBgp_Signal 242
#define GObjUID_CBgpGA_RollVendor 243
#define GObjUID_RollVendorResult 244
#define GObjUID_CBgpGA_CheckAward 245
#define GObjUID_CLevelSpell_FireBall 246
#define GObjUID_CBgpGA_RollSpells 247
#define GObjUID_CBgp_ResPile 248
#define GObjUID_CBgpGA_ModSP 249
#define GObjUID_CLevelAbility_FlameBlade 250
#define GObjUID_CLevelAbility_LightningBow 251
#define GObjUID_DamagesEx 252
#define GObjUID_DamageEx 253
#define GObjUID_ResistEx 254
#define GObjUID_ResistsEx 255
#define GObjUID_EvadeEx 256
#define GObjUID_EvadesEx 257
#define GObjUID_HitEx 258
#define GObjUID_HitsEx 259
#define GObjUID_CLevelAbility_HonorSword 260
#define GObjUID_CLevelAbility_SkullSword 261
#define GObjUID_CBgp_Delay 262
#define GObjUID_CLevelAbility_TeleportSword 263
#define GObjUID_CLevelAbility_PhantomDagger 264
#define GObjUID_CLevelAbility_BloodTeeth 265
#define GObjUID_CLevelAbility_ObliterateBow 266
#define GObjUID_MiscFlagEx 267
#define GObjUID_CLevelAbility_WolfSkin 268
#define GObjUID_CLevelAbility_TalBless 269
#define GObjUID_CLevelAbility_AnWeep 270
#define GObjUID_CLevelAbility_BlackSteel 271
#define GObjUID_CLevelAbility_HunterPlate 272
#define GObjUID_CLevelAbility_SimCurse 273
#define GObjUID_CLevelAbility_WhirlWind 274
#define GObjUID_CLevelAbility_HonorPlate 275
#define GObjUID_CLevelAbility_7sonLeather 276
#define GObjUID_CLevelAbility_Frost 277
#define GObjUID_CLevelAbility_EliPromise 278
#define GObjUID_PPosUpd_Tornado 279
#define GObjUID_PPosSpnTornado 280
#define GObjUID_AttackModEx 281
#define GObjUID_AttackModsEx 282
#define GObjUID_DefendModEx 283
#define GObjUID_DefendModsEx 284
#define GObjUID_CBgpTroop_SwitchRetinue 285
#define GObjUID_CBgpThreat_FindFleePos 286
#define GObjUID_CBgpGA_ModShape 287
#define GObjUID_CBgpSetupSlatesA_SetEntrance 288
#define GObjUID_CBgpSetupSlatesA_SetType 289
#define GObjUID_CBgpSlatesA_FinishProcess 290
#define GObjUID_SlatesRandomPickEntryA 291
#define GObjUID_CBgpSetupSlatesA_SetType_RandomPick 292
#define GObjUID_CBgpSlatesA_CheckProcessed 293
#define GObjUID_CBgpSlatesA_Reveal 294
#define GObjUID_LevelSlatesGroup 295
#define GObjUID_CBgpSetupSlatesA_SetMatchKey 296
#define GObjUID_CBgpSlatesA_Teleport 297
#define GObjUID_CBgpSlatesA_SpawnAgent 298
#define GObjUID_CBgpSlatesA_OpenFence 299
#define GObjUID_CBgpSetupSlatesA_SetEdgeLock 300
#define GObjUID_CBgpSetupSlatesA_SetSwitch 301
#define GObjUID_CBgpSetupSlatesA_SetButton 302
#define GObjUID_CLevelAbility_ExplodeOil 303
#define GObjUID_CLevelAbility_WeaponInductionStone 304
#define GObjUID_LevelRecordEo 305
#define GObjUID_EoParamBullet 306
#define GObjUID_CBgp_CheckGrade 307
#define GObjUID_CLevelAbility_ToeStone 308
#define GObjUID_CLevelAbility_SacredArrow 309
#define GObjUID_CLevelAbility_Bomb 310
#define GObjUID_CBgp_SetUnitPace 311
#define GObjUID_CLevelAbility_MagicRing 312
#define GObjUID_CLevelAbility_MoneyBag 313
#define GObjUID_CLevelAbility_GemPot 314
#define GObjUID_CLevelAbility_HPAmulet 315
#define GObjUID_CLevelAbility_SPAmulet 316
#define GObjUID_CLevelAbility_HPPotion 317
#define GObjUID_CLevelAbility_SPPotion 318
#define GObjUID_CLevelAbility_VampireRing 319
#define GObjUID_CLevelAbility_ShieldMask 320
#define GObjUID_CLevelAbility_Whetstone 321
#define GObjUID_CLevelAbility_HonorBook 322
#define GObjUID_CBgpGA_AssignVita 323
#define GObjUID_CLevelAbility_ShieldAmulet 324
#define GObjUID_CBgpRtnu_Accompany 325
#define GObjUID_CBgp_CheckGuardAttack_Obsolete 326
#define GObjUID_CBgpThreat_KeepDist 327
#define GObjUID_CBgpRtnu_CheckBehavior 328
#define GObjUID_LichenParam 329
#define GObjUID_CBgpGA_Env 330
#define GObjUID_CBgpLichen 331
#define GObjUID_CBgp_CheckDead 332
#define GObjUID_CBgpAI_CheckMentalState 333
#define GObjUID_CBgpAI_SetMentalState 334
#define GObjUID_CBgpCmd_Monitor 335
#define GObjUID_CBgpThreat_Combating 336
#define GObjUID_CBgpThreat_Alerting 337
#define GObjUID_CBgpThreat_CheckCombatedCount 338
#define GObjUID_CBgpThreat_CheckAlertedCount 339
#define GObjUID_CBgpThreat_Siege 340
#define GObjUID_CBgp_KeepCheck 341
#define GObjUID_CBgp_Always 342
#define GObjUID_CBgp_Roll 343
#define GObjUID_CBgpThreat_FindAttackPos 344
#define GObjUID_WeaksEx 345
#define GObjUID_OppressesEx 346
#define GObjUID_CBgp_CheckMoving 347
#define GObjUID_CBgp_CanCancelSkill 348
#define GObjUID_CBgp_CheckStunSrc 349
#define GObjUID_CBgp_WaitStun 350
#define GObjUID_CLevelAbility_StormEye 351
#define GObjUID_BlockingEx 352
#define GObjUID_BlockingConvertsEx 353
#define GObjUID_CBgp_CheckSkillWindow 354
#define GObjUID_SkillParam_General_ZMatch 355
#define GObjUID_CBgp_CheckSkillStage 356
#define GObjUID_CBgp_SwitchSkillStage 357
#define GObjUID_CBgp_SkillOp 358
#define GObjUID_CBgp_Jink 359
#define GObjUID_CBgp_AttackTargetPos 360
#define GObjUID_CBgp_CheckFlies 361
#define GObjUID_EoParamChainedHammer 362
#define GObjUID_CBgp_ChainedHammerOp 363
// #define GObjUID_CBgp_Ð¡¶ñÄ§_Ñ°ÕÒÉÁ±ÜÌøÔ¾µã 364
#define GObjUID_CBgp_WeaksMod 365
#define GObjUID_CBgp_MakeSkillStun 366
#define GObjUID_CBgp_CheckStunInfo 367
#define GObjUID_CBgpTroop_Join 368
#define GObjUID_CBgp_´´½¨·ÉÑý 369
#define GObjUID_CBgp_CheckSkillTargetRange 370
#define GObjUID_CBgp_·ÉÑý_¸üÐÂFlankAttack 371
#define GObjUID_CBgp_Deal 372
#define GObjUID_CBgp_·ÉÑý_¸üÐÂShuttle 373
#define GObjUID_CBgpSetSkillTarget 374
#define GObjUID_CBgp_FindWalkableArea 375
#define GObjUID_CBgp_CheckVar_ID 376
#define GObjUID_BgnLevelSkillTarget 377
#define GObjUID_CBgp_AttackTargetFace 378
#define GObjUID_CBgp_CentipedeNode_Activate 379
#define GObjUID_CBgpThreat_ForceUpdate 380
#define GObjUID_CBgp_GetSkillDynObstacle 381
#define GObjUID_CBgp_Possess 382
#define GObjUID_CBgp_Centipede_SpawnCyst 383
#define GObjUID_CBgp_CentipedeNode_GetPrevNode 384
#define GObjUID_CBgpThreat_UpdateCtrl 385
#define GObjUID_CBgp_FindNearbyService 386
#define GObjUID_CBgp_PreserveService 387
#define GObjUID_CBgp_DiscardService 388
#define GObjUID_CBgp_CheckPreserveService 389
#define GObjUID_CBgp_GetServiceServerCount 390
#define GObjUID_CBgp_RemoveService 391
#define GObjUID_CBgpThreat_CalcDist 392
#define GObjUID_CBgp_CanPreserveService 393
#define GObjUID_CBgp_GetPreserveServer 394
#define GObjUID_CBgp_Ensure 395
#define GObjUID_CBgp_CheckSkillStageAge 396
#define GObjUID_CBgp_Centipede_GetInfo 397
#define GObjUID_CBgp_Centipede_Stretch 398
// #define GObjUID_CBgp_SnailP1Ã¼Ðë_Ñ°ÕÒ¹¥»÷µã 399
#define GObjUID_CBgp_CalcFace 400
// #define GObjUID_CBgp_SnailP1Ã¼Ðë_³õÊ¼»¯ 401
// #define GObjUID_CBgp_SnailP1Éà³æ_¹¤×÷Ä£Ê½ 402
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400
// #define GObjUID_ 400


#define BEGIN_GOBJ_UID(classtype,version)											\
BEGIN_GOBJ(classtype,version)																\
GObj<classtype>::__uid()=GOBJ_UID_START+GObjUID_##classtype;

#define BEGIN_GOBJ_UID2(classtype,uid,version)											\
	BEGIN_GOBJ(classtype,version)																\
	GObj<classtype>::__uid()=GOBJ_UID_START+(uid);

#define BEGIN_GOBJ_PURE_UID(classtype,version)								\
BEGIN_GOBJ_PURE(classtype,version)														\
GObj<classtype>::__uid()=GOBJ_UID_START+GObjUID_##classtype;

#define BEGIN_GOBJ_PURE_UID2(classtype,uid,version)								\
	BEGIN_GOBJ_PURE(classtype,version)														\
	GObj<classtype>::__uid()=GOBJ_UID_START+(uid);


#define BEGIN_GOBJ_PURE_NESTED_UID2(classtype,fullclasstype,uid,version)										\
	BEGIN_GOBJ_PURE_NESTED(classtype,fullclasstype,version)														\
	GObj<classtype>::__uid()=GOBJ_UID_START+(uid);
