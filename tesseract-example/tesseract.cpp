#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

tesseract::TessBaseAPI *api;
tesseract::WritingDirection direction;
  
void cleanupMemory(char *outText, Pix *image){
	// Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);
}

void setupTesseract(){
	api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with Arabic
	printf("Start tesseract Initialize:\n");
    if (api->Init("/home/pi/Desktop/project/sample/tesseract-example/tessdata", "ara")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }
	printf("End tesseract Initialize:\n");
}

int main()
{
    setupTesseract();    
    Pix *image = pixRead("/home/pi/Desktop/project/sample/tesseract-example/arabic2.png");
    api->SetImage(image);
    
    // Get OCR result
    char *outText;
    outText = api->GetUTF8Text();
    printf("OCR output:\n%s", outText);

    cleanupMemory(outText, image);
    
    return 0;
}
