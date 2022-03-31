#include "Sprite.h"
#include "TextureManager.h"
#include <cassert>
#include <d3dcompiler.h>
#include <d3dx12.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

/// <summary>
/// 静的メンバ変数の実体
/// </summary>
ID3D12Device* Sprite::sDevice = nullptr;
UINT Sprite::sDescriptorHandleIncrementSize;
ID3D12GraphicsCommandList* Sprite::sCommandList = nullptr;
ComPtr<ID3D12RootSignature> Sprite::sRootSignature;
ComPtr<ID3D12PipelineState> Sprite::sPipelineState;
XMMATRIX Sprite::sMatProjection;

void Sprite::StaticInitialize(ID3D12Device* device, int window_width, int window_height) {
  // nullptrチェック
  assert(device);

  sDevice = device;

  // デスクリプタサイズを取得
  sDescriptorHandleIncrementSize =
    sDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  HRESULT result = S_FALSE;
  ComPtr<ID3DBlob> vsBlob;    // 頂点シェーダオブジェクト
  ComPtr<ID3DBlob> psBlob;    // ピクセルシェーダオブジェクト
  ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

  // 頂点シェーダの読み込みとコンパイル
  result = D3DCompileFromFile(
    L"Resources/shaders/SpriteVS.hlsl", // シェーダファイル名
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
    "main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
    0, &vsBlob, &errorBlob);
  if (FAILED(result)) {
	// errorBlobからエラー内容をstring型にコピー
	std::string errstr;
	errstr.resize(errorBlob->GetBufferSize());

	std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
	errstr += "\n";
	// エラー内容を出力ウィンドウに表示
	OutputDebugStringA(errstr.c_str());
	exit(1);
  }

  // ピクセルシェーダの読み込みとコンパイル
  result = D3DCompileFromFile(
    L"Resources/shaders/SpritePS.hlsl", // シェーダファイル名
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
    "main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
    0, &psBlob, &errorBlob);
  if (FAILED(result)) {
	// errorBlobからエラー内容をstring型にコピー
	std::string errstr;
	errstr.resize(errorBlob->GetBufferSize());

	std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
	errstr += "\n";
	// エラー内容を出力ウィンドウに表示
	OutputDebugStringA(errstr.c_str());

	exit(1);
  }

  // 頂点レイアウト
  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
    {// xy座標(1行で書いたほうが見やすい)
     "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {// uv座標(1行で書いたほうが見やすい)
     "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  // グラフィックスパイプラインの流れを設定
  D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
  gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
  gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

  // サンプルマスク
  gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
  // ラスタライザステート
  gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
  // デプスステンシルステート
  gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // 常に上書きルール

  // レンダーターゲットのブレンド設定
  D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
  blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA全てのチャンネルを描画
  blenddesc.BlendEnable = true;
  blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
  blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
  blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

  blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
  blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
  blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

  // ブレンドステートの設定
  gpipeline.BlendState.RenderTarget[0] = blenddesc;

  // 深度バッファのフォーマット
  gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

  // 頂点レイアウトの設定
  gpipeline.InputLayout.pInputElementDescs = inputLayout;
  gpipeline.InputLayout.NumElements = _countof(inputLayout);

  // 図形の形状設定（三角形）
  gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

  gpipeline.NumRenderTargets = 1;                            // 描画対象は1つ
  gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
  gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

  // デスクリプタレンジ
  CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
  descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

  // ルートパラメータ
  CD3DX12_ROOT_PARAMETER rootparams[2] = {};
  rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
  rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

  // スタティックサンプラー
  CD3DX12_STATIC_SAMPLER_DESC samplerDesc =
    CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR); // s0 レジスタ
  samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

  // ルートシグネチャの設定
  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init_1_0(
    _countof(rootparams), rootparams, 1, &samplerDesc,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> rootSigBlob;
  // バージョン自動判定のシリアライズ
  result = D3DX12SerializeVersionedRootSignature(
    &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
  assert(SUCCEEDED(result));
  // ルートシグネチャの生成
  result = sDevice->CreateRootSignature(
    0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
    IID_PPV_ARGS(&sRootSignature));
  assert(SUCCEEDED(result));

  gpipeline.pRootSignature = sRootSignature.Get();

  // グラフィックスパイプラインの生成
  result = sDevice->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&sPipelineState));
  assert(SUCCEEDED(result));

  // 射影行列計算
  sMatProjection = XMMatrixOrthographicOffCenterLH(
    0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f);
}

void Sprite::PreDraw(ID3D12GraphicsCommandList* commandList) {
  // PreDrawとPostDrawがペアで呼ばれていなければエラー
  assert(Sprite::sCommandList == nullptr);

  // コマンドリストをセット
  sCommandList = commandList;

  // パイプラインステートの設定
  sCommandList->SetPipelineState(sPipelineState.Get());
  // ルートシグネチャの設定
  sCommandList->SetGraphicsRootSignature(sRootSignature.Get());
  // プリミティブ形状を設定
  sCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::PostDraw() {
  // コマンドリストを解除
  Sprite::sCommandList = nullptr;
}

Sprite* Sprite::Create(
  uint32_t textureHandle, XMFLOAT2 position, XMFLOAT4 color, XMFLOAT2 anchorpoint, bool isFlipX,
  bool isFlipY) {
  // 仮サイズ
  XMFLOAT2 size = {100.0f, 100.0f};

  {
	// テクスチャ情報取得
	const D3D12_RESOURCE_DESC& resDesc =
	  TextureManager::GetInstance()->GetResoureDesc(textureHandle);
	// スプライトのサイズをテクスチャのサイズに設定
	size = {(float)resDesc.Width, (float)resDesc.Height};
  }

  // Spriteのインスタンスを生成
  Sprite* sprite = new Sprite(textureHandle, position, size, color, anchorpoint, isFlipX, isFlipY);
  if (sprite == nullptr) {
	return nullptr;
  }

  // 初期化
  if (!sprite->Initialize()) {
	delete sprite;
	assert(0);
	return nullptr;
  }

  return sprite;
}

Sprite::Sprite() {}

Sprite::Sprite(
  uint32_t textureHandle, XMFLOAT2 position, XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 anchorpoint,
  bool isFlipX, bool isFlipY) {
  position_ = position;
  size_ = size;
  anchorPoint_ = anchorpoint;
  matWorld_ = XMMatrixIdentity();
  color_ = color;
  textureHandle_ = textureHandle;
  isFlipX_ = isFlipX;
  isFlipY_ = isFlipY;
  texSize_ = size;
}

bool Sprite::Initialize() {
  // nullptrチェック
  assert(sDevice);

  HRESULT result = S_FALSE;

  resourceDesc_ = TextureManager::GetInstance()->GetResoureDesc(textureHandle_);

  {
	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc =
	  CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv) * kVertNum);

	// 頂点バッファ生成
	result = sDevice->CreateCommittedResource(
	  &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	  IID_PPV_ARGS(&vertBuff_));
	assert(SUCCEEDED(result));

	// 頂点バッファマッピング
	result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
  }

  // 頂点バッファへのデータ転送
  TransferVertices();

  // 頂点バッファビューの作成
  vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
  vbView_.SizeInBytes = sizeof(VertexPosUv) * 4;
  vbView_.StrideInBytes = sizeof(VertexPosUv);

  {
	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc =
	  CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff);

	// 定数バッファの生成
	result = sDevice->CreateCommittedResource(
	  &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	  IID_PPV_ARGS(&constBuff_));
	assert(SUCCEEDED(result));
  }

  // 定数バッファマッピング
  result = constBuff_->Map(0, nullptr, (void**)&constMap);
  assert(SUCCEEDED(result));

  return true;
}

void Sprite::SetTextureHandle(uint32_t textureHandle) {
  textureHandle_ = textureHandle;
  resourceDesc_ = TextureManager::GetInstance()->GetResoureDesc(textureHandle_);
}

void Sprite::SetRotation(float rotation) {
  rotation_ = rotation;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::SetPosition(const DirectX::XMFLOAT2& position) {
  position_ = position;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::SetSize(const DirectX::XMFLOAT2& size) {
  size_ = size;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::SetAnchorPoint(const DirectX::XMFLOAT2& anchorpoint) {
  anchorPoint_ = anchorpoint;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::SetIsFlipX(bool isFlipX) {
  isFlipX_ = isFlipX;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::SetIsFlipY(bool isFlipY) {
  isFlipY_ = isFlipY;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::SetTextureRect(const DirectX::XMFLOAT2& texBase, const DirectX::XMFLOAT2& texSize) {
  texBase_ = texBase;
  texSize_ = texSize;

  // 頂点バッファへのデータ転送
  TransferVertices();
}

void Sprite::Draw() {
  // ワールド行列の更新
  matWorld_ = XMMatrixIdentity();
  matWorld_ *= XMMatrixRotationZ(rotation_);
  matWorld_ *= XMMatrixTranslation(position_.x, position_.y, 0.0f);

  // 定数バッファにデータ転送
  constMap->color = color_;
  constMap->mat = matWorld_ * sMatProjection; // 行列の合成

  // 頂点バッファの設定
  sCommandList->IASetVertexBuffers(0, 1, &vbView_);

  // 定数バッファビューをセット
  sCommandList->SetGraphicsRootConstantBufferView(0, constBuff_->GetGPUVirtualAddress());
  // シェーダリソースビューをセット
  TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, 1, textureHandle_);
  // 描画コマンド
  sCommandList->DrawInstanced(4, 1, 0, 0);
}

void Sprite::TransferVertices() {
  HRESULT result = S_FALSE;

  // 左下、左上、右下、右上
  enum { LB, LT, RB, RT };

  float left = (0.0f - anchorPoint_.x) * size_.x;
  float right = (1.0f - anchorPoint_.x) * size_.x;
  float top = (0.0f - anchorPoint_.y) * size_.y;
  float bottom = (1.0f - anchorPoint_.y) * size_.y;
  if (isFlipX_) { // 左右入れ替え
	left = -left;
	right = -right;
  }

  if (isFlipY_) { // 上下入れ替え
	top = -top;
	bottom = -bottom;
  }

  // 頂点データ
  VertexPosUv vertices[kVertNum];

  vertices[LB].pos = {left, bottom, 0.0f};  // 左下
  vertices[LT].pos = {left, top, 0.0f};     // 左上
  vertices[RB].pos = {right, bottom, 0.0f}; // 右下
  vertices[RT].pos = {right, top, 0.0f};    // 右上

  // テクスチャ情報取得
  {
	float tex_left = texBase_.x / resourceDesc_.Width;
	float tex_right = (texBase_.x + texSize_.x) / resourceDesc_.Width;
	float tex_top = texBase_.y / resourceDesc_.Height;
	float tex_bottom = (texBase_.y + texSize_.y) / resourceDesc_.Height;

	vertices[LB].uv = {tex_left, tex_bottom};  // 左下
	vertices[LT].uv = {tex_left, tex_top};     // 左上
	vertices[RB].uv = {tex_right, tex_bottom}; // 右下
	vertices[RT].uv = {tex_right, tex_top};    // 右上
  }

  // 頂点バッファへのデータ転送
  memcpy(vertMap, vertices, sizeof(vertices));
}
