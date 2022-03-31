#include "Audio.h"
#include "DirectXCommon.h"
#include "GameScene.h"
#include "TextureManager.h"
#include "WinApp.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  WinApp* win = nullptr;
  DirectXCommon* dxCommon = nullptr;
  // 汎用機能
  Input* input = nullptr;
  Audio* audio = nullptr;
  DebugText* debugText = nullptr;
  GameScene* gameScene = nullptr;

  // ゲームウィンドウの作成
  win = new WinApp();
  win->CreateGameWindow();

  // DirectX初期化処理
  dxCommon = new DirectXCommon();
  dxCommon->Initialize(win);

#pragma region 汎用機能初期化
  // 入力の初期化
  input = new Input();
  input->Initialize(win->GetInstance(), win->GetHwnd());

  // オーディオの初期化
  audio = Audio::GetInstance();
  audio->Initialize();

  // テクスチャマネージャの初期化
  TextureManager::GetInstance()->Initialize(dxCommon->GetDevice());
  TextureManager::Load("white1x1.png");

  // スプライト静的初期化
  Sprite::StaticInitialize(dxCommon->GetDevice(), WinApp::kWindowWidth, WinApp::kWindowHeight);

  // デバッグテキスト初期化
  debugText = new DebugText();
  debugText->Initialize();

  // 3Dオブジェクト静的初期化
  Model::StaticInitialize(dxCommon->GetDevice(), WinApp::kWindowWidth, WinApp::kWindowHeight); 
  #pragma endregion

  // ゲームシーンの初期化
  gameScene = new GameScene();
  gameScene->Initialize(dxCommon, input, audio, debugText);

  // メインループ
  while (true) {
	// メッセージ処理
	if (win->ProcessMessage()) {
	  break;
	}

	// 入力関連の毎フレーム処理
	input->Update();
	// ゲームシーンの毎フレーム処理
	gameScene->Update();

	// 描画開始
	dxCommon->PreDraw();
	// ゲームシーンの描画
	gameScene->Draw();
	// 描画終了
	dxCommon->PostDraw();
  }

  // 各種解放
  SafeDelete(gameScene);
  SafeDelete(debugText);
  audio->Finalize();
  SafeDelete(input);
  SafeDelete(dxCommon);

  // ゲームウィンドウの破棄
  win->TerminateGameWindow();
  SafeDelete(win);

  return 0;
}