package idg.ezio;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }
    private Button btn_s;   // stitcher
    private Button btn_c;   // colour
    private Button btn_g;
    private ImageView imageViewS; // show
    private Bitmap bitmap_s;
    private Bitmap bitmap_l;
    private Bitmap bitmap_r;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        bitmap_s = BitmapFactory.decodeResource(getResources(),R.drawable.color);
        bitmap_l = BitmapFactory.decodeResource(getResources(),R.drawable.left);
        bitmap_r = BitmapFactory.decodeResource(getResources(),R.drawable.right);

        btn_s = findViewById(R.id.buttonS);
        btn_s.setOnClickListener(this);
        btn_c = findViewById(R.id.buttonC);
        btn_c.setOnClickListener(this);
        btn_g = findViewById(R.id.buttonG);
        btn_g.setOnClickListener(this);

        imageViewS = findViewById(R.id.imageViewS);
    }

    public void show(){
        bitmap_s = BitmapFactory.decodeResource(getResources(),R.drawable.color);
        imageViewS.setImageBitmap(bitmap_s);
    }

    public void gray(){
        int w = bitmap_s.getWidth();
        int h = bitmap_s.getHeight();
        int[] pixels = new int[w*h];
        bitmap_s.getPixels(pixels,0,w,0,0,w,h);
        int[] resultData =Bitmap2Grey(pixels,w,h);
        Bitmap resultImage = Bitmap.createBitmap(w,h, Bitmap.Config.ARGB_8888);
        resultImage.setPixels(resultData,0,w,0,0,w,h);
        imageViewS.setImageBitmap(resultImage);
    }

    public void stitch(){
        int w = bitmap_l.getWidth();
        int h = bitmap_l.getHeight();
        int[] fir = new int[w*h];
        int[] sec = new int[w*h];
        bitmap_l.getPixels(fir,0,w,0,0,w,h);
        bitmap_r.getPixels(sec,0,w,0,0,w,h);
        int[] resultData = StitchImages(fir, sec, w, h);
        w = resultData[0];
        h = resultData[1];
        Bitmap resultImage = Bitmap.createBitmap(w,h, Bitmap.Config.ARGB_8888);
        resultImage.setPixels(resultData,0,w,0,0,w,h);
        imageViewS.setImageBitmap(resultImage);
    }
    @Override
    public void onClick(View view){
        switch(view.getId()){
            case R.id.buttonC: show();break;
            case R.id.buttonG: gray();break;
            case R.id.buttonS: stitch(); break;
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
 //   public native String stringFromJNI();
    public native int[] Bitmap2Grey(int[] pixels,int w,int h);
    public native int[] StitchImages(int[] fir, int[] sec, int w, int h);
}
