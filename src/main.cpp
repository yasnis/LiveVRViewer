#include "ofMain.h"
#include "ofxGui.h"
#include "ofxBlackMagic.h"
#include "ofxSyphon.h"
#include "OSXWindow.h"

class ofApp : public ofBaseApp{
    enum AppMode {
        AppMode_Release,
        AppMode_Debug
    };
    
    AppMode mode = AppMode_Debug;
    ofxPanel gui;
    ofParameterGroup param_group0;
    ofParameter<float> param_group0_param0;
    ofParameter<int> param_group0_param1;
    ofParameter<int> param_group0_param2;
    
    ofParameterGroup param_group1;
    ofParameter<bool> param_group1_param0;
    ofParameter<string> param_group1_param1;
    ofxButton param_group1_param2;
    
    ofxBlackmagic::Input *input;
    
//    ofVideoGrabber *input;
    
    vector<shared_ptr<ofxBlackmagic::Input> > inputs;
    
    ofSpherePrimitive sphere;
    
    ofPlanePrimitive plane;
    
//    ofEasyCam camera;
    ofCamera camera;
    ofParameter<float> cameraTheta = 0.0;
    ofParameter<float> cameraPhi = 0.0;
    ofParameter<float> cameraZoom = 68.0;
//    float cameraZoom = 68.0;
    ofVec3f cameraTarget;
    
    
    ofVec2f mouse;
    
    bool drawSphere = true;
    ofParameter<bool> drawPlane = true;
    ofParameter<bool> useSyphon = false;
    ofParameter<bool> showOnTop = false;
//    ofParameter<bool> use4K = true;
    
    float lastClick;
    
    ofxSyphonClient client;
    ofTexture texture;
    ofFbo fbo;
    ofMatrix4x4 mat;
    
    Window window;
    
    //--------------------------------------------------------------
    void setup(){
        ofBackground(0);
        ofSetFrameRate(60);
        ofSetVerticalSync(true);
        
        setupGUI();
        setupPrimitives();
        
        //*
        auto deviceList = ofxBlackmagic::Iterator::getDeviceList();
        if (deviceList.size()>0) {
            input = new ofxBlackmagic::Input();
            cout << deviceList[0].modelName << endl;
            
            if(deviceList[0].modelName == "UltraStudio Mini Recorder"){
                input->startCapture(deviceList[0], bmdModeHD1080p30);
            }else{
                input->startCapture(deviceList[0], bmdMode4K2160p30);
            }
        }
        
        cout << camera.getFov() << endl;
        lastClick = ofGetElapsedTimeMillis();
        
        windowResized(0, 0);
        
        client.setup();
        
        mat.scale(1, -1, 1);
        mat.translate(0, 1, 0);
        
        window.setWindowOnTop(showOnTop);
    }
    
    void setupPrimitives() {
        sphere.set(100, 16);
    }
    void windowResized(int w, int h){
        plane.set(ofGetWidth()/2, ofGetWidth()/4);
        plane.setPosition(ofVec3f(ofGetWidth()/2,ofGetHeight()-ofGetWidth()/8));
        
        ofRectangle r = gui.getShape();
        r.setPosition(r.x, ofGetHeight()-r.height-10);
        gui.setShape(r);
    }
    
    void setupGUI() {
        gui.setDefaultWidth(320);
        gui.setup();
        gui.setName("Setting");
        param_group0.setName("Camera Setting.");
        param_group0.add(cameraTheta.set("Hedding(LEFT/RIGHT)", 0, -M_PI, M_PI));
        param_group0.add(cameraPhi.set("Pitch(UP/DOWN)", 0, -M_PI_2, M_PI_2));
        
        param_group1.setName("App Setting.");
        param_group1.add(useSyphon.set("Use Syphon(s)", false));
        param_group1.add(drawPlane.set("Draw Texture Data(t)", false));
        param_group1.add(showOnTop.set("Show Window On Top(w)", false));
//        param_group1.add(use4K.set("Use 4K(Restart required.)", false));
        
        gui.add(param_group1);
        gui.add(param_group0);
        
        gui.loadFromFile("settings.xml");
        
        showOnTop.addListener(this, &ofApp::updateBoolParam);
        
    }
    
    //--------------------------------------------------------------
    void update(){
        
        if(ofGetMousePressed()){
            float dx = mouseX-mouse.x;
            float dy = mouseY-mouse.y;
            cout <<mouseY << " : " << -mouse.y << endl;
            addCameraTargetPosition(dx*0.0001, -dy*0.0001);
        }
        
        
        cameraTarget.y = sin(cameraPhi);
        cameraTarget.x = cos(cameraPhi)*cos(cameraTheta-M_PI_2);
        cameraTarget.z = cos(cameraPhi)*sin(cameraTheta-M_PI_2);
        
        camera.lookAt(cameraTarget);
        camera.setFov(cameraZoom);
        
        if(input){
            input->update();
            texture = input->getTexture();
        }
        
        if(useSyphon){
            client.bind();
            texture = client.getTexture();
            texture.setTextureMatrix(mat);
            client.unbind();
        }
        
    }
    
    
    void addCameraTargetPosition(float theta, float phi){
        cameraTheta+=theta;
        if(cameraTheta<-M_PI){
            cameraTheta+=M_PI*2;
        }else if(cameraTheta>M_PI){
            cameraTheta-=M_PI*2;
        }
        cameraPhi+=phi;
        cameraPhi = MAX(MIN(cameraPhi.get(), M_PI_2), -M_PI_2);
    }
    
    void addCameraZoom(float zoom){
        cameraZoom+=zoom;
        cameraZoom = MAX(MIN(cameraZoom.get(), 170), 5);
    }
    
    //--------------------------------------------------------------
    void draw() {
        if(!input){
            ofSetColor(255, 0, 0);
            string str = "UltraStudio 4K not found.";
            ofDrawBitmapString(str, 20, 20);
            ofSetColor(255);
        }
        //*
        if(drawSphere) {
            camera.begin();
            texture.bind();
            ofPushMatrix();
            ofRotate(180, 0, 0, 1);
            sphere.draw();
            ofPopMatrix();
            texture.unbind();
            camera.end();
        }
        if(drawPlane) {
            texture.bind();
            plane.draw();
            texture.unbind();
        }
        //*/
        if(mode == AppMode_Debug) {
            ofDisableNormalizedTexCoords();
            drawDebug();
            string str = "------------------------------\n";
            if(!input){
                str += "UltraStudio not found.\n";
                str += "------------------------------\n";
            }
            str += "Heading(rad) : LEFT, RIGHT\n";
            str += "Pitch(rad) : UP, DOWN\n";
//            str += "Field of View(degree) : a, z\n";
            str += "Reset Camera Position : Double Click\n";
            str += "------------------------------\n";
            str += "Use Syphon : s\n";
            str += "Draw Texture Data : t\n";
            str += "Show Window On Top : w\n";
            str += "------------------------------\n";
            str += "Show Parameter : d\n";
            str += "Toggle Fullscreen : f\n";
            str += "------------------------------\n";
            ofDrawBitmapString(str, 20, 20);
            ofEnableNormalizedTexCoords();
        }
    }
    
    void drawDebug() {
        ofSetColor(255);
        gui.draw();
    }
    
    void exit() {
        gui.saveToFile("settings.xml");
    }
    
    void switchDebug() {
        if (mode == AppMode_Debug) {
            mode = AppMode_Release;
        }else{
            mode = AppMode_Debug;
        }
    }
    
    void resetCamera() {
        cameraTheta = 0;
        cameraPhi = 0;
//        cameraZoom = 60;
    }
    
    void keyPressed(int key){
        if(key == 'f'){
            ofToggleFullscreen();
        }else if(key == 'd') {
            switchDebug();
        }else if(key == 't') {
            drawPlane = !drawPlane;
        }else if(key == OF_KEY_LEFT){
            addCameraTargetPosition(-0.05, 0);
        }else if(key == OF_KEY_RIGHT){
            addCameraTargetPosition(+0.05, 0);
        }else if(key == OF_KEY_UP){
            addCameraTargetPosition(0, 0.05);
        }else if(key == OF_KEY_DOWN){
            addCameraTargetPosition(0, -0.05);
        }else if(key == 'a'){
//            addCameraZoom(-1.0);
        }else if(key == 'z'){
//            addCameraZoom(1.0);
        }else if(key == 's'){
            useSyphon = !useSyphon;
        }else if(key == 'w'){
            showOnTop = !showOnTop;
            window.setWindowOnTop(showOnTop);
        }
    }
    
    void updateFloatParam(float &value) {
        //do something.
        cout << "updateFloatParam : " << value << endl;
    }
    
    void updateIntParam(int &value) {
        //do something.
        cout << "updateIntParam : " << value << endl;
    }
    
    void mouseMoved(int x, int y){
        
    }
    
    //--------------------------------------------------------------
    void mouseDragged(int x, int y, int button){
        float dx = x-mouse.x;
        float dy = y-mouse.y;
    }
    
    //--------------------------------------------------------------
    void mousePressed(int x, int y, int button){
        mouse.x = x;
        mouse.y = y;
        
        if(ofGetElapsedTimeMillis() - lastClick < 300 && button == 0){
            mouseDoubleClicked(x, y, button);
        }
        lastClick = ofGetElapsedTimeMillis();
    }
    
    void mouseDoubleClicked(int x, int y, int button){
        cout << "mouseDoubleClicked" << endl;
        resetCamera();
    }
    
    //--------------------------------------------------------------
    void mouseReleased(int x, int y, int button){
        
    }
    void mouseScrolled( ofMouseEventArgs & mouse ){
        cout << mouse.scrollY << endl;
        addCameraZoom(mouse.scrollY);
//        cameraZoom+=mouse.scrollY;
    }
    
    
    void updateBoolParam(bool &value) {
        window.setWindowOnTop(value);
    }
};

//========================================================================
int main( ){
	ofSetupOpenGL(960,540, OF_WINDOW);
	ofRunApp( new ofApp());
}
