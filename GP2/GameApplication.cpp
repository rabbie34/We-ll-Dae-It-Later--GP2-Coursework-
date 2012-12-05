#include "GameApplication.h"
#include "GameObject.h"
#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Joypad.h"
#include <ctime>

CGameApplication::CGameApplication(void)
{
	m_pWindow=NULL;
	m_pD3D10Device=NULL;
	m_pRenderTargetView=NULL;
	m_pSwapChain=NULL;
	m_pDepthStencelView=NULL;
	m_pDepthStencilTexture=NULL;
	m_pGameObjectManager=new CGameObjectManager();
	m_GameState=MAINMENU;
	m_fCurrentTime=0.0f;
}

CGameApplication::~CGameApplication(void)
{
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	CGUIManager::getInstance().destroy();

	if (m_pGameObjectManager)
	{
		delete m_pGameObjectManager;
		m_pGameObjectManager=NULL;
	}

	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if (m_pDepthStencelView)
		m_pDepthStencelView->Release();
	if (m_pDepthStencilTexture)
		m_pDepthStencilTexture->Release();
	if (m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pD3D10Device)
		m_pD3D10Device->Release();
	if (m_pWindow)
	{
		delete m_pWindow;
		m_pWindow=NULL;
	}
}

bool CGameApplication::init()
{
	if (!initWindow())
		return false;
	if (!initGraphics())
		return false;
	if (!initInput())
		return false;
	if(!initGUI())
		return false;
	if (!initAudio())
		return false;
	if (!initGame())
		return false;
	return true;
}

bool CGameApplication::initGUI()
{
	D3D10_VIEWPORT vp;
	UINT numViewports=1;
	m_pD3D10Device->RSGetViewports(&numViewports,&vp);
	CGUIManager::getInstance().init(m_pD3D10Device,vp.Width,vp.Height);
	return true;
}

void CGameApplication::initMainGame()
{
	// Set primitive topology, how are we going to interpet the vertices in the vertex buffer - BMD
    //http://msdn.microsoft.com/en-us/library/bb173590%28v=VS.85%29.aspx - BMD
    m_pD3D10Device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );	

	//Creation of Skybox
	CGameObject *pTestGameObject=new CGameObject();
	pTestGameObject->setName("Sky");
	CMeshComponent *pMesh=modelloader.createCube(m_pD3D10Device,2.0f,2.0f,2.0f);
	pMesh->SetRenderingDevice(m_pD3D10Device);
	CMaterialComponent *pMaterial=new CMaterialComponent();
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Environment.fx");
	pMaterial->loadEnvironmentTexture("Space.dds");
	pTestGameObject->addComponent(pMaterial);
	pTestGameObject->addComponent(pMesh);
	//Sound for flying through ring
	CAudioSourceComponent *pSmash=new CAudioSourceComponent();
	pSmash->setFilename("Smashing.wav"); //If its a wav file, you should not stream
	pSmash->setStream(false); //stream set to false
	pTestGameObject->addComponent(pSmash); //Add it to the Game Object
	m_pGameObjectManager->addGameObject(pTestGameObject);

	//Set up for the player ship. Loads in an object, textures it and sets position/rotation.
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("player");
	pTestGameObject->getTransform()->setPosition(0.0f,0.0f,-20.0f);
	pTestGameObject->getTransform()->setScale(0.2f,0.2f,0.2f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("buffShip_Diff.jpg");
	pMaterial->loadSpecularTexture("buffShip_Spec.jpg");
	//pMaterial->loadParallaxTexture("buffShip_parallax.jpg");  //Textures dont work properly.
	//pMaterial->loadBumpTexture("buffShip_Bump.png");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"buffship2.fbx","buffshipfix");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	//Sounds for the player ship
	CAudioSourceComponent *pLaser=new CAudioSourceComponent();
	pLaser->setFilename("laser.wav");
	pLaser->setStream(false);
	pTestGameObject->addComponent(pLaser);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	//Sets the ship speed and rotation
	shipRot = m_pGameObjectManager->findGameObject("player")->getTransform()->getRotation();
	speed=12.0f;
	rotSpeed=18.0f;

	//Creation of various planets and objects to popluate the scene. Uses the same method as player creation.
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Earth");
	pTestGameObject->getTransform()->setPosition(-150.0f,160.0f,500.0f);
	pTestGameObject->getTransform()->setRotation(1.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(0.8f,0.8f,0.8f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_plan.bmp");
	pMaterial->loadBumpTexture("mat_plan NORMAL.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"planet_earth.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	//Sound for being hit by asteroid
	CAudioSourceComponent *pBlarg=new CAudioSourceComponent();
	pBlarg->setFilename("Blarg.wav");
	pBlarg->setStream(false);
	pTestGameObject->addComponent(pBlarg);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("PlanetIce");
	pTestGameObject->getTransform()->setPosition(150.0f,-90.0f,650.0f);
	pTestGameObject->getTransform()->setRotation(1.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(0.6f,0.6f,0.6f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_plan2.bmp");
	pMaterial->loadBumpTexture("mat_plan NORMAL2.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"planet_ice.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("PlanetFire");
	pTestGameObject->getTransform()->setPosition(300.0f,400.0f,900.0f);
	pTestGameObject->getTransform()->setRotation(1.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(1.5f,1.5f,1.5f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_plan3.bmp");
	pMaterial->loadBumpTexture("mat_plan NORMAL3.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"planet_fire.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Moon");
	pTestGameObject->getTransform()->setPosition(-120.0f,160.0f,500.0f);
	pTestGameObject->getTransform()->setRotation(1.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(0.15f,0.15f,0.15f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_plan4.bmp");
	pMaterial->loadBumpTexture("mat_plan NORMAL4.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"planet_moon.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Jupiter");
	pTestGameObject->getTransform()->setPosition(-150.0f,-130.0f,380.0f);
	pTestGameObject->getTransform()->setRotation(1.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(1.5f,1.5f,1.5f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_plan5.bmp");
	pMaterial->loadBumpTexture("mat_plan NORMAL5.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"planet_jupiter.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Mars");
	pTestGameObject->getTransform()->setPosition(250.0f,-230.0f,450.0f);
	pTestGameObject->getTransform()->setRotation(1.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(1.0f,1.0f,1.0f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_plan6.bmp");
	pMaterial->loadBumpTexture("mat_plan NORMAL6.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"planet_mars.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);

	//Creation of the space gate that the player will fly through
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Gate");
	pTestGameObject->getTransform()->setPosition(0.0f,0.0f,5.0f);
	pTestGameObject->getTransform()->setScale(0.15f,0.15f,0.15f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_gate.bmp");
	pMaterial->loadBumpTexture("mat_gate NORMAL.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"gate.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	
	//Asteroid that the player can collide with
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Asteroid");
	pTestGameObject->getTransform()->setPosition(0.0f,0.0f,300.0f);
	pTestGameObject->getTransform()->setRotation(0.0f,0.0f,0.0f);
	pTestGameObject->getTransform()->setScale(0.07f,0.07f,0.07f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_ateroid_1.bmp");
	pMaterial->loadBumpTexture("mat_asteNORMAL.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"asteroid.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	//Sound for aseroid exlode
	CAudioSourceComponent *pExplode=new CAudioSourceComponent();
	pExplode->setFilename("Asteroid.wav");
	pExplode->setStream(false);
	pTestGameObject->addComponent(pExplode);
	m_pGameObjectManager->addGameObject(pTestGameObject);

	//Creation of the space station and satellites that surround it.
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Station");
	pTestGameObject->getTransform()->setPosition(0.0f,-400.0f,600.0f);
	pTestGameObject->getTransform()->setRotation(1.6f,0.0f,1.6f);
	pTestGameObject->getTransform()->setScale(1.0f,1.0f,1.0f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_stat.bmp");
	pMaterial->loadBumpTexture("mat_stat NORMAL.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"station.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Satellite");
	pTestGameObject->getTransform()->setPosition(0.0f,-350.0f,650.0f);
	pTestGameObject->getTransform()->setScale(0.1f,0.1f,0.1f);
	pTestGameObject->getTransform()->setRotation(1.6f,1.0f,1.5f);
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Parallax.fx");
	pMaterial->setAmbientMaterialColour(D3DXCOLOR(0.2f,0.2f,0.2f,1.0f));
	pMaterial->loadDiffuseTexture("mat_sate.bmp");
	pMaterial->loadBumpTexture("mat_stateNORMAL.bmp");
	pTestGameObject->addComponent(pMaterial);
	pMesh=modelloader.loadModelFromFile(m_pD3D10Device,"satellite.fbx","");
	pMesh->SetRenderingDevice(m_pD3D10Device);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Satellite2");
	pTestGameObject->getTransform()->setPosition(100.0f,-480.0f,500.0f);
	pTestGameObject->getTransform()->setScale(0.1f,0.1f,0.1f);
	pTestGameObject->getTransform()->setRotation(1.6f,1.0f,1.5f);
	pTestGameObject->addComponent(pMaterial);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);
	pTestGameObject=new CGameObject();
	pTestGameObject->setName("Satellite3");
	pTestGameObject->getTransform()->setPosition(-120.0f,-400.0f,600.0f);
	pTestGameObject->getTransform()->setScale(0.1f,0.1f,0.1f);
	pTestGameObject->getTransform()->setRotation(1.6f,1.0f,1.5f);
	pTestGameObject->addComponent(pMaterial);
	pTestGameObject->addComponent(pMesh);
	m_pGameObjectManager->addGameObject(pTestGameObject);

	//Creation and positioning of the camera
	CTransformComponent * pTransform=m_pGameObjectManager->findGameObject("player")->getTransform();
	D3DXVECTOR3 coords = pTransform->getPosition();
	CGameObject *pCameraGameObject=new CGameObject();
	pCameraGameObject->getTransform()->setPosition(coords.x,coords.y+2.0f,coords.z-15.0f);
	pCameraGameObject->setName("Camera");
	D3D10_VIEWPORT vp;
	UINT numViewports=1;
	m_pD3D10Device->RSGetViewports(&numViewports,&vp);
	CCameraComponent *pCamera=new CCameraComponent();
	pCamera->setUp(0.0f,1.0f,0.0f);
	pCamera->setLookAt(coords.x,coords.y,coords.z);
	pCamera->setFOV(D3DX_PI*0.25f);
	pCamera->setAspectRatio((float)(vp.Width/vp.Height));
	pCamera->setFarClip(1000.0f);
	pCamera->setNearClip(0.1f);
	pCameraGameObject->addComponent(pCamera);
	//Audio listener for the camera and the game music
	CAudioListenerComponent *pListener=new CAudioListenerComponent();
	pCameraGameObject->addComponent(pListener);
	CAudioSourceComponent *pMusic=new CAudioSourceComponent();
	pMusic->setFilename("Star Wars.mp3");
	pMusic->setStream(true);
	pCameraGameObject->addComponent(pMusic);
	m_pGameObjectManager->addGameObject(pCameraGameObject);

	//directional light for the whole scene
	CGameObject *pLightGameObject=new CGameObject();
	pLightGameObject->setName("DirectionalLight");
	CDirectionalLightComponent *pLightComponent=new CDirectionalLightComponent();
	pLightComponent->setDirection(D3DXVECTOR3(0.0f,1.0f,-1.0f));
	pLightGameObject->addComponent(pLightComponent);
	m_pGameObjectManager->addGameObject(pLightGameObject);
	m_pGameObjectManager->setMainLight(pLightComponent);

	//init, this must be called after we have created all game objects
	m_pGameObjectManager->init();

	//Play the game music and thruster sounds
	pMusic->play(-1);
	//pAudio->play(-1);

	score=0; //set the player score at the start to 0
	srand((unsigned)time(0)); //Seed used for random number generation
	
	m_Timer.start();
}

void CGameApplication::initMainMenu()
{
	m_pGameObjectManager->clear();

	//Creation of Skybox
	CGameObject *pTestGameObject=new CGameObject();
	pTestGameObject->setName("Sky");
	CMeshComponent *pMesh=modelloader.createCube(m_pD3D10Device,2.0f,2.0f,2.0f);
	pMesh->SetRenderingDevice(m_pD3D10Device);
	CMaterialComponent *pMaterial=new CMaterialComponent();
	pMaterial=new CMaterialComponent();
	pMaterial->SetRenderingDevice(m_pD3D10Device);
	pMaterial->setEffectFilename("Environment.fx");
	pMaterial->loadEnvironmentTexture("Space.dds");
	pTestGameObject->addComponent(pMaterial);
	pTestGameObject->addComponent(pMesh);
	//Sound for flying through ring
	CAudioSourceComponent *pSmash=new CAudioSourceComponent();
	pSmash->setFilename("Smashing.wav"); //If its a wav file, you should not stream
	pSmash->setStream(false); //stream set to false
	pTestGameObject->addComponent(pSmash); //Add it to the Game Object
	m_pGameObjectManager->addGameObject(pTestGameObject);


	//Create Mesh
	CGameObject *pCameraGameObject=new CGameObject();
	pCameraGameObject->getTransform()->setPosition(0.0f,0.0f,-5.0f);
	pCameraGameObject->setName("Camera");

	D3D10_VIEWPORT vp;
	UINT numViewports=1;
	m_pD3D10Device->RSGetViewports(&numViewports,&vp);

	CCameraComponent *pCamera=new CCameraComponent();
	pCamera->setUp(0.0f,1.0f,0.0f);
	pCamera->setLookAt(0.0f,0.0f,0.0f);
	pCamera->setFOV(D3DX_PI*0.25f);
	pCamera->setAspectRatio((float)(vp.Width/vp.Height));
	pCamera->setFarClip(1000.0f);
	pCamera->setNearClip(0.1f);
	pCameraGameObject->addComponent(pCamera);

	m_pGameObjectManager->addGameObject(pCameraGameObject);


	m_pMainMenu=CGUIManager::getInstance().loadGUI("mainMenu.rml");
	m_pGameGUI=CGUIManager::getInstance().loadGUI("GameGUI.rml");
	m_pPauseGUI=CGUIManager::getInstance().loadGUI("PauseGUI.rml");
	m_pMainMenu->Show();

	//init, this must be called after we have created all game objects
	m_pGameObjectManager->init();
}

bool CGameApplication::initGame()
{
    // Set primitive topology, how are we going to interpet the vertices in the vertex buffer - BMD
    //http://msdn.microsoft.com/en-us/library/bb173590%28v=VS.85%29.aspx - BMD
    m_pD3D10Device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );	
	//initMainGame();
	initMainMenu();
	//m_pMainMenu->Show();

	m_Timer.start();
	
	return true;
}

void CGameApplication::run()
{
	while(m_pWindow->running())
	{
		if (! m_pWindow->checkForWindowMessages())
		{
			update();
			render();
		}
	}
}

void CGameApplication::render()
{
    // Just clear the backbuffer, colours start at 0.0 to 1.0
	// Red, Green , Blue, Alpha - BMD
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; 
	//Clear the Render Target
	//http://msdn.microsoft.com/en-us/library/bb173539%28v=vs.85%29.aspx - BMD
    m_pD3D10Device->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pD3D10Device->ClearDepthStencilView(m_pDepthStencelView,D3D10_CLEAR_DEPTH,1.0f,0);
	//We need to iterate through all the Game Objects in the managers
	for(vector<CGameObject*>::iterator iter=m_pGameObjectManager->getBegining();iter!=m_pGameObjectManager->getEnd();iter++)
	{
		//grab the transform
		CTransformComponent *pTransform=(*iter)->getTransform();
		//and the geometry
		CMeshComponent *pMesh=static_cast<CMeshComponent*>((*iter)->getComponent("MeshComponent"));
		//and the material
		CMaterialComponent *pMaterial=static_cast<CMaterialComponent*>((*iter)->getComponent("MaterialComponent"));

		//do we have a matrial
		if (pMaterial)
		{
			CCameraComponent *camera=m_pGameObjectManager->getMainCamera();

			//set the matrices
			pMaterial->setProjectionMatrix((float*)camera->getProjection());
			pMaterial->setViewMatrix((float*)camera->getView());
			pMaterial->setWorldMatrix((float*)pTransform->getWorld());
			//set light colour
			pMaterial->setAmbientLightColour(D3DXCOLOR(0.5f,0.5f,0.5f,1.0f));

			//get the main light and the camera
			CDirectionalLightComponent * light=m_pGameObjectManager->getMainLight();
			pMaterial->setDiffuseLightColour(light->getDiffuseColour());
			pMaterial->setSpecularLightColour(light->getSpecularColour());
			pMaterial->setLightDirection(light->getLightDirection());
			
			pMaterial->setCameraPosition(camera->getParent()->getTransform()->getPosition());

			pMaterial->setTextures();
			pMaterial->setMaterial();
			//bind the vertex layout
			pMaterial->bindVertexLayout();
			//loop for the passes in the material
			for (UINT i=0;i<pMaterial->getNumberOfPasses();i++)
			{
				//Apply the current pass
				pMaterial->applyPass(i);
				//we have a geometry
				if (pMesh)
				{
					//Loop through all the subsets in the mesh
					for (int i=0;i<pMesh->getTotalNumberOfSubsets();i++)
					{
						//grab one of the subset
						CGeometry *pSubset=pMesh->getSubset(i);
						//bind the buffers contained in the subset
						pSubset->bindBuffers();
						//draw
						m_pD3D10Device->DrawIndexed(pSubset->getNumberOfIndices(),0,0);
					}
				}
			}
		}
		CGUIManager::getInstance().render();
	}
	//Swaps the buffers in the chain, the back buffer to the front(screen)
	//http://msdn.microsoft.com/en-us/library/bb174576%28v=vs.85%29.aspx - BMD
    m_pSwapChain->Present( 0, 0 );
}

void CGameApplication::updateMainGame()
{
	m_pGameGUI->Show();
	//Used to update the game, physics simulation, audio and joypad input.
	CAudioSystem::getInstance().update();
	CInput::getInstance().getJoypad(0)->update();

	bool gameplaying=true; //Variable used to get around editing more than one object at a time and game states.

	//Get the position of the ship to be used in various methods
	CTransformComponent * pTransform=m_pGameObjectManager->findGameObject("player")->getTransform();
	//Move the ship forward constantly even if its rotation changes.
	D3DXVECTOR3 direction = pTransform->getForward();
	pTransform->translate(direction.x* m_Timer.getElapsedTime() * speed, direction.y*m_Timer.getElapsedTime(), direction.z * m_Timer.getElapsedTime() * speed );
	//Rotate the ship back to its original position if no key is pressed
	pTransform->rotate((shipRot.x-pTransform->getRotation().x)*m_Timer.getElapsedTime()*5.0f,(shipRot.y-pTransform->getRotation().y)*m_Timer.getElapsedTime()*5.0f,(shipRot.z-pTransform->getRotation().z)*m_Timer.getElapsedTime()*5.0f);

	//rotates Objects in the scene continuously.
	if(gameplaying=true)
	{
		//Rotation on the space gate
		CTransformComponent * pTransform2=m_pGameObjectManager->findGameObject("Gate")->getTransform();
		pTransform2->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*1.25f);

		//Rotation of the various planets in the scene. Also moves them away from the player so they can never reach them.
		CTransformComponent * pTransform3=m_pGameObjectManager->findGameObject("Earth")->getTransform();
		pTransform3->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*0.10f);
		pTransform3->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform4=m_pGameObjectManager->findGameObject("PlanetIce")->getTransform();
		pTransform4->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*0.15f);
		pTransform4->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform5=m_pGameObjectManager->findGameObject("PlanetFire")->getTransform();
		pTransform5->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*0.08f);
		pTransform5->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform6=m_pGameObjectManager->findGameObject("Moon")->getTransform();
		pTransform6->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*0.15f);
		pTransform6->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform7=m_pGameObjectManager->findGameObject("Jupiter")->getTransform();
		pTransform7->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*0.04f);
		pTransform7->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform8=m_pGameObjectManager->findGameObject("Mars")->getTransform();
		pTransform8->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*0.07f);
		pTransform8->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);

		//Makes the moon orbit the earth
		float xPos = pTransform3->getPosition().x+sin(m_Timer.getTotalTime()/4)*55;
		float yPos = pTransform3->getPosition().y;
		float zPos = pTransform3->getPosition().z+cos(m_Timer.getTotalTime()/4)*55;
		pTransform6->setPosition(xPos,yPos,zPos);

		//movement of the space station and satellites away from the player so they can never reach them.
		CTransformComponent * pTransform9=m_pGameObjectManager->findGameObject("Station")->getTransform();
		pTransform9->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform10=m_pGameObjectManager->findGameObject("Satellite")->getTransform();
		pTransform10->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform11=m_pGameObjectManager->findGameObject("Satellite2")->getTransform();
		pTransform11->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);
		CTransformComponent * pTransform12=m_pGameObjectManager->findGameObject("Satellite3")->getTransform();
		pTransform12->translate(0.0f,0.0f,m_Timer.getElapsedTime()*speed);

		//move the asteroid towards the player and have it spinning. If it goes past the player, respawn it back infront.
		CTransformComponent * pTransform13=m_pGameObjectManager->findGameObject("Asteroid")->getTransform();
		pTransform13->translate(0.0f,0.0f,m_Timer.getElapsedTime()*-100.0f);
		pTransform13->rotate(m_Timer.getElapsedTime()*-5.0f,m_Timer.getElapsedTime(),0.0f);
		if(pTransform13->getPosition().z<pTransform->getPosition().z-10.0f)
		{
			pTransform13->setPosition(pTransform->getPosition().x,pTransform->getPosition().y,pTransform->getPosition().z+300.0f);
		}
	}

	//make the camera follow the ship around the screen by setting its lookat to the ship co-ordinates 
	//and its position close to those same co-ords.
	if(gameplaying=true)
	{
		CTransformComponent * pTransform2=m_pGameObjectManager->findGameObject("Camera")->getTransform();
		CCameraComponent * pCamera=m_pGameObjectManager->getMainCamera();
		pCamera->setLookAt(pTransform->getPosition().x,pTransform->getPosition().y,pTransform->getPosition().z);
		pTransform2->setPosition(pTransform->getPosition().x,pTransform->getPosition().y+0.0f,pTransform->getPosition().z-15.0f);
	}

	//When the player presses the mouse/trigger down check to see if they hit an asteroid and play a sound.
	if (CInput::getInstance().getMouse()->getMouseDown(0)|| CInput::getInstance().getJoypad(0)->getRightTrigger()>0)
	{
		CAudioSourceComponent * pLaser=(CAudioSourceComponent *)m_pGameObjectManager->findGameObject("player")->getComponent("AudioSourceComponent");
		if (audioTimer < 0.0f)
		{
			pLaser->play();
			audioTimer = 0.4f;
		}
		CTransformComponent * pTransform2=m_pGameObjectManager->findGameObject("Asteroid")->getTransform();
		if(CInput::getInstance().getMouse()->getRelativeMouseX()<=pTransform2->getPosition().x+2.5f && CInput::getInstance().getMouse()->getRelativeMouseX()>=pTransform2->getPosition().x-6.5f)
		{
			CAudioSourceComponent * pExplode=(CAudioSourceComponent *)m_pGameObjectManager->findGameObject("Asteroid")->getComponent("AudioSourceComponent");
			pExplode->play();
			pTransform2->setPosition(pTransform->getPosition().x,pTransform->getPosition().y,pTransform->getPosition().z+300.0f);
		}
	}
	//Ensures the sound is not played too often.
	if (audioTimer >= 0)
	{
		audioTimer-=m_Timer.getElapsedTime();
	}

	//Fly the ship up/down/left/right depending on they key pressed/the direction of the joystick. 
	//Titls the ship in the direction of movement as well as changes its actual positioning.
	if (CInput::getInstance().getKeyboard()->isKeyDown((int)'W') || CInput::getInstance().getJoypad(0)->getLeftThumbStickY()<-12000)
	{
		pTransform->translate(0.0f,m_Timer.getElapsedTime()*rotSpeed,0.0f);
		pTransform->rotate(m_Timer.getElapsedTime()*-2.0f,0.0f,0.0f);	
	}
	else if (CInput::getInstance().getKeyboard()->isKeyDown((int)'S') || CInput::getInstance().getJoypad(0)->getLeftThumbStickY()>12000)
	{
		pTransform->translate(0.0f,m_Timer.getElapsedTime()*-rotSpeed,0.0f);
		pTransform->rotate(m_Timer.getElapsedTime()*1.0f,0.0f,0.0f);
	}
	if (CInput::getInstance().getKeyboard()->isKeyDown((int)'D') || CInput::getInstance().getJoypad(0)->getLeftThumbStickX()>12000)
	{
		pTransform->translate(m_Timer.getElapsedTime()*rotSpeed,0.0f,0.0f);
		pTransform->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*-2.5f);
	}
	else if (CInput::getInstance().getKeyboard()->isKeyDown((int)'A') || CInput::getInstance().getJoypad(0)->getLeftThumbStickX()<-12000)
	{
		pTransform->translate(m_Timer.getElapsedTime()*-rotSpeed,0.0f,0.0f);
		pTransform->rotate(0.0f,0.0f,m_Timer.getElapsedTime()*2.5f);	
	}

	//Increase the ship speed when space/trigger is held down, then set the speed back to normal when nothing is being pressed.
	if (CInput::getInstance().getKeyboard()->isKeyDown(VK_SPACE) || CInput::getInstance().getJoypad(0)->getLeftTrigger()>0.5)
	{
		if(speed<35.0f)
		{
			speed=speed+m_Timer.getElapsedTime()*10.0f;
			rotSpeed=rotSpeed+m_Timer.getElapsedTime()*10.0f;
		}
	}
	else
	{
		if(speed>12.0f)
		{
			speed=speed-m_Timer.getElapsedTime()*10.0f;
			rotSpeed=rotSpeed-m_Timer.getElapsedTime()*10.0f;
		}
		//Ensures the speed can not go below its original value.
		if(speed<12.0f)
		{
			speed=12.0f;
			rotSpeed=18.0f;
		}
	}

	//Gets a random position on x and y within the ships range to move the space gate too.
	float random = RandomFloat ((pTransform->getPosition().x-30.0f),(pTransform->getPosition().x+30.0f));
	float random2 = RandomFloat ((pTransform->getPosition().y-30.0f),(pTransform->getPosition().y+30.0f));
	float random3 = RandomFloat ((pTransform->getPosition().z+35.0f),(pTransform->getPosition().z+100.0f));

	//Method used for collision detection with the space gates and asteroids.
	if(gameplaying=true)
	{
		//Get the co-ordinates of the space gate and check to see if the ship passes through it.
		CTransformComponent * pTransform2=m_pGameObjectManager->findGameObject("Gate")->getTransform();
		if(pTransform->getPosition().y<=pTransform2->getPosition().y+4.0f && pTransform->getPosition().y>=pTransform2->getPosition().y-4.0f)
		{
			if(pTransform->getPosition().x<=pTransform2->getPosition().x+3.5f && pTransform->getPosition().x>=pTransform2->getPosition().x-3.5f)
			{
				if(pTransform->getPosition().z>pTransform2->getPosition().z+6.0f && pTransform->getPosition().z<pTransform2->getPosition().z+7.0f )
				{
				//if the player passes through the gate then move it forward to a random position and increase the score depending on their speed
				pTransform2->setPosition(random,random2,random3);
				score=score+speed;
				CAudioSourceComponent * pSmash=(CAudioSourceComponent *)m_pGameObjectManager->findGameObject("Sky")->getComponent("AudioSourceComponent");
				pSmash->play();
				}
			}
		}
		//If the ship missed the space gate then move the space gate anyway
		if(pTransform->getPosition().z>pTransform2->getPosition().z+20.0f)
		{
			pTransform2->setPosition(random,random2,pTransform2->getPosition().z+150.0f);
		}
		
		//Get the co-ordinates of the asteroid and check to see if the ship hits it.
		CTransformComponent * pTransform3=m_pGameObjectManager->findGameObject("Asteroid")->getTransform();
		if(pTransform->getPosition().y<=pTransform3->getPosition().y+3.0f && pTransform->getPosition().y>=pTransform3->getPosition().y-3.0f)
		{
			if(pTransform->getPosition().x<=pTransform3->getPosition().x+2.5f && pTransform->getPosition().x>=pTransform3->getPosition().x-6.5f)
			{
				if(pTransform->getPosition().z>pTransform3->getPosition().z && pTransform->getPosition().z<pTransform3->getPosition().z+1.0f )
				{
					float rotX = RandomFloat (-1,1);
					float rotY = RandomFloat (-1,1);
					float rotZ = RandomFloat (-1,1);
					CAudioSourceComponent * pBlarg=(CAudioSourceComponent *)m_pGameObjectManager->findGameObject("Earth")->getComponent("AudioSourceComponent");
					pBlarg->play();
					pTransform3->setPosition(pTransform->getPosition().x,pTransform->getPosition().y,pTransform->getPosition().z+300.0f);
					pTransform->rotate(rotX,rotY,rotZ);
					pTransform->translate(rotX*5,rotY*5,rotZ*5);
					score=0;
				}
			}
		}
	}

		if (CInput::getInstance().getKeyboard()->isKeyDown((int)'P')||CInput::getInstance().getJoypad(0)->isButtonPressed(0x00000010))
		{
			if (m_GameState==GAME){
				m_pGameGUI->Hide();
				m_GameState=PAUSE;
				m_pPauseGUI->Show();
			}
		}

	m_pGameObjectManager->update(m_Timer.getElapsedTime());
}

void CGameApplication::updateMainMenu()
{
	CInput::getInstance().getJoypad(0)->update();
	if (CInput::getInstance().getKeyboard()->isKeyDown(VK_RETURN)||CInput::getInstance().getJoypad(0)->isButtonPressed(0x1000))
	{
		m_pMainMenu->Hide();
		m_GameState=GAME;
		initMainGame();
	}
}

void CGameApplication::updatePauseGUI()
{
	CInput::getInstance().getJoypad(0)->update();
		if(CInput::getInstance().getKeyboard()->isKeyDown((int)'O')||CInput::getInstance().getJoypad(0)->isButtonPressed(0x2000))
		{
			if(m_GameState==PAUSE)
			{
				m_pPauseGUI->Hide();
				m_GameState=GAME;		
				m_pGameGUI->Show();
			}
		}
}

void CGameApplication::update()
{
	m_Timer.update();
	m_fCurrentTime+=m_Timer.getElapsedTime();
	switch (m_GameState)
	{
		case MAINMENU:
			{
				updateMainMenu();
				break;
			}
		case GAME:
			{
				updateMainGame();
				break;
			}
		case PAUSE:
			{
				updatePauseGUI();
				break;
			}
	}

	m_pGameObjectManager->update(m_Timer.getElapsedTime());
}

bool CGameApplication::initInput()
{
	CInput::getInstance().init();
	return true;
}

//initGraphics - initialise the graphics subsystem - BMD
bool CGameApplication::initGraphics()
{
	//Retrieve the size of the window, this is need to match the
	//back buffer to screen size - BMD
	RECT windowRect;
	//http://msdn.microsoft.com/en-us/library/ms633503%28v=vs.85%29.aspx -BMD
	GetClientRect(m_pWindow->getHandleToWindow(),&windowRect);

	//Calculate the width and height of the window - BMD
	UINT width=windowRect.right-windowRect.left;
	UINT height=windowRect.bottom-windowRect.top;

	//Device creation flags, used to control our the D3D10 device is created
	UINT createDeviceFlags=0;
	//If we are in a debug build then set the device creation flag to debug device
#ifdef _DEBUG
	createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif

	//Swap Chain description - used in the creation of the swap chain
	//http://msdn.microsoft.com/en-us/library/bb173075%28v=vs.85%29.aspx - BMD

	//Initialise the swap chain description by setting all its values to zero - BMD
	DXGI_SWAP_CHAIN_DESC sd;
	//http://msdn.microsoft.com/en-us/library/aa366920%28v=vs.85%29.aspx - BMD
    ZeroMemory( &sd, sizeof( sd ) );
	//What kind of surface is contained in the swap chain, in this case something we draw too
	//http://msdn.microsoft.com/en-us/library/bb173078%28v=vs.85%29.aspx - BMD
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//Number of buffers, if we are not full screen this will be one as the desktop
	//acts as a front buffer. If we are in full screen this will be one - BMD
	if (m_pWindow->isFullScreen())
		sd.BufferCount = 2;
	else 
		sd.BufferCount=1;
	//The handle of the window which this swap chain is linked to, this must not be NULL - BMD
	sd.OutputWindow = m_pWindow->getHandleToWindow();
	//Are we in windowed mode, arggh opposite of full screen
	sd.Windowed = (BOOL)(!m_pWindow->isFullScreen());
	//Multisampling(antialsing) parameters for the swap chain - this has performance considerations - see remarks in docs
	//http://msdn.microsoft.com/en-us/library/bb173072%28v=vs.85%29.aspx - BMD
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
	//The description of the swap chain buffer
	//http://msdn.microsoft.com/en-us/library/bb173064%28v=vs.85%29.aspx - BMD
	//width & height of the buffer - this matches the size of the window - BMD
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
	//The data format of the buffer in the swap chain, 8bits used for Red, green, blue & alpha - unsigned int(UNIFORM) - BMD
	//http://msdn.microsoft.com/en-us/library/bb173059%28v=vs.85%29.aspx
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//Refresh rate of the buffer in the swap chain - BMD
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
	
	//NB. You should get use to seeing patterns like this when programming with D3D10 
	//where we use a description object which is then used in the creation of a D3D10 resource 
	//like swap chains. Also in a real application we would check to see if some of the above
	//options are support by the graphics hardware. -BMD

	//Create D3D10 Device and swap chain 
	//http://msdn.microsoft.com/en-us/library/bb205087%28v=vs.85%29.aspx - BMD
	if (FAILED(D3D10CreateDeviceAndSwapChain(NULL, //Pointer to IDXGIAdpater, this is a display adapater on the machine this can be NULL - BMD
		D3D10_DRIVER_TYPE_HARDWARE,//Type of Driver we have, it can be a hardware device, refrence(slow) or Software(not supported yet) - BMD
		NULL, //Handle to a module that implements a software rasterizer - BMD
		createDeviceFlags,//The device creation flags we used earlier on - BMD
		D3D10_SDK_VERSION,//The version of the SDK we are using this should D3D10 - BMD
		&sd,//The memory address of the swap chain description - BMD
		&m_pSwapChain, //The memory address of the swap chain pointer, if all goes well this will be intialised after this function call - BMD
		&m_pD3D10Device)))//the memory address of the D3D10 Device, if all goes well this will be initialised after this function call - BMD
		return false;

	//NB. There are two ways of creating the device, the above way which initialises the device and swap chain at the sametime
	// or we can create a swap chain and a device seperatly and then associate a swap chain with a device. - BMD

	//Create a render target, this is a Texture which will hold our backbuffer, this will
	//enable us to link the rendertarget with buffer held in the swap chain - BMD
	ID3D10Texture2D *pBackBuffer;
	//Get a buffer from the swap chain 
	//http://msdn.microsoft.com/en-us/library/bb174570%28v=vs.85%29.aspx - BMD
	if (FAILED(m_pSwapChain->GetBuffer(0, //buffer index, 0 will get the back buffer
		__uuidof(ID3D10Texture2D),//The unique identifier of the type of pointer we want in
								  //this case a I3D10 Texture2D
		(void**)&pBackBuffer)))//A pointer to a memory address, this is cast to a void ** because this function
							   //can return back different types dependent on the 2nd param
		return false;

	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width=width;
	descDepth.Height=height;
	descDepth.MipLevels=1;
	descDepth.ArraySize=1;
	descDepth.Format=DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count=1;
	descDepth.SampleDesc.Quality=0;
	descDepth.Usage=D3D10_USAGE_DEFAULT;
	descDepth.BindFlags=D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags=0;
	descDepth.MiscFlags=0;

	if (FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,&m_pDepthStencilTexture)))
		return false;

	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format=descDepth.Format;
	descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice=0;

	if (FAILED(m_pD3D10Device->CreateDepthStencilView(m_pDepthStencilTexture,&descDSV,&m_pDepthStencelView)))
		return false;


	//Create the Render Target View, a view is the way we access D3D10 resources
	//http://msdn.microsoft.com/en-us/library/bb173556%28v=vs.85%29.aspx - BMD
	if (FAILED(m_pD3D10Device->CreateRenderTargetView( pBackBuffer, //The resource we are creating the view for - BMD
		NULL, //The description of the view, in this case NULL - BMD
		&m_pRenderTargetView ))) // the memory address of a pointer to D3D10 Render Target - BMD
	{
		
		pBackBuffer->Release();
		return  false;
	}
	//The above Get Buffer call will allocate some memory, we now need to release it. - BMD
    pBackBuffer->Release();

	//Binds one or more render targets and depth buffer to the Output merger stage - BMD
	//http://msdn.microsoft.com/en-us/library/bb173597%28v=vs.85%29.aspx - BMD
	m_pD3D10Device->OMSetRenderTargets(1, //Number  of views - BMD
		&m_pRenderTargetView, //pointer to an array of D3D10 Render Target Views - BMD
		m_pDepthStencelView); //point to Depth Stencil buffer - BMD

    // Setup the viewport 
	//http://msdn.microsoft.com/en-us/library/bb172500%28v=vs.85%29.aspx - BMD
    D3D10_VIEWPORT vp;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
	//Sets the Viewport 
	//http://msdn.microsoft.com/en-us/library/bb173613%28v=vs.85%29.aspx - BMD
    m_pD3D10Device->RSSetViewports( 1 //Number of viewports to bind
		, &vp );//an array of viewports

	return true;
}

bool CGameApplication::initAudio()
{
	CAudioSystem::getInstance().init();
	return true;
}

bool CGameApplication::initWindow()
{
	m_pWindow=new CWin32Window();
	if (!m_pWindow->init(TEXT("Games Programming"),800,640,false))
		return false;
	return true;
}