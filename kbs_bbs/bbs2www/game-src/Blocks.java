//****************************************************************************

// ---- general information ----

//

// BLOCKS.JAVA           v 1.01 13 jan. '96

// Written by :          I. van Rienen / E-mail ivr@bart.nl

// URL:                  http://www/bart.nl/~ivr

// Initial release:      17 dec. '95

// Released as freeware: 05 jan. '96 v 1.00

//

// ----- version information

// v 1.00   17 dec. '95 Initial Release

// v 1.01   13 jan. '96 Arrow-keys changed to fix sun problem.

//

// ---- Description ----

// JAVA program for playing BLOCKS on Internet

//

// This program is postcard-ware.

// If you like this program, send a postcard to:

//

//    Iwan van Rienen

//    J. Maetsuyckerstr. 145

//    2593 ZG  The Hague

//    The Netherlands

//

// Thanks in advance!

// you are free to do anything you want with this program.

// However, I am not responsible for any bugs in this program and

// possible damage to hard- or software when using this program.

// Please refer to this program when you are using it in one of your programs

// Feel free to e-mail me at any time at ivr@bart.nl

// Visit my homepage at http://www.bart.nl/~ivr for more software

//****************************************************************************



import java.awt.*;

import java.awt.image.*;

import java.applet.Applet;

import java.applet.AudioClip;

import java.util.Vector;

import java.lang.Math;

import java.lang.Thread;

import java.lang.System;



public class Blocks extends java.applet.Applet implements Runnable {

    public static final int cols = 10;

    public static final int rows = 18;

    public static final int ElementSize = 15;

    public static final int MaxElement = 3;

    public static final Color BackGroundColor = Color.black;

    public static final int SOUND_DROP = 0;

    public static final int SOUND_GAMEOVER = 1;

    public static final int SOUND_NEXTLEVEL = 2;

    public static final int SOUND_LINE = 3;



    PlayFieldCanvas myPlayFieldCanvas;

    StatisticsCanvas Statistics;

    Thread killme = null;

    Vector ShapeSet;

    Shape FallingShape = null;

    Shape NextShape = null;

    Color PlayField[][];

    static AudioClip sounds[];

    public boolean GamePaused = false;



    public void init() {

        setLayout(new BorderLayout());

        Panel grid = new Panel();

        grid.setLayout(new GridLayout(0, 2));



        add("Center", grid);

        myPlayFieldCanvas = new PlayFieldCanvas();

        grid.add(myPlayFieldCanvas);

        Statistics = new StatisticsCanvas();

        grid.add(Statistics);



        Panel p = new Panel();

        p.setLayout(new FlowLayout());

        add("East", p);

        p.add(new Button("About"));

        p.add(new Button("Pause / Resume"));



        ShapeSet = new Vector();

        ShapeSet.addElement(new Shape(  0x0000 ,

                                        0x0FF0 ,

                                        0x0FF0 ,

                                        0x0000 , Color.blue, 3));



        ShapeSet.addElement(new Shape(  0x0F00 ,

                                        0x0F00 ,

                                        0x0FF0 ,

                                        0x0000 , Color.yellow, 5));



        ShapeSet.addElement(new Shape(  0x00F0 ,

                                        0x00F0 ,

                                        0x0FF0 ,

                                        0x0000 , Color.pink, 5));



        ShapeSet.addElement(new Shape(  0x0000 ,

                                        0x0F00 ,

                                        0xFFF0 ,

                                        0x0000 , Color.green, 4));



        ShapeSet.addElement(new Shape(  0x0F00 ,

                                        0x0F00 ,

                                        0x0F00 ,

                                        0x0F00 , Color.red, 4));



        ShapeSet.addElement(new Shape(  0x0F00 ,

                                        0x0FF0 ,

                                        0x00F0 ,

                                        0x0000 , Color.magenta, 4));



        ShapeSet.addElement(new Shape(  0x00F0 ,

                                        0x0FF0 ,

                                        0x0F00 ,

                                        0x0000 , Color.orange, 4));



        sounds = new AudioClip[4];

        sounds[0] = getAudioClip(getCodeBase(), "audio/drop.au");

        sounds[1] = getAudioClip(getCodeBase(), "audio/gameover.au");

        sounds[2] = getAudioClip(getCodeBase(), "audio/nextlevel.au");

        sounds[3] = getAudioClip(getCodeBase(), "audio/line.au");



        PlayField = new Color[cols][rows];

        myPlayFieldCanvas.SetPlayField(PlayField);

        InitNewGame();

        GetNextRandomShape();

    }



    public static void play(int n) {

        if (sounds[n] != null) {

            sounds[n].play();

        }

    }



    public void InitNewGame() {

        ClearPlayField();

        Statistics.InitNewGame();

    }



    public void ClearPlayField() {

        int x, y;



        for (x = 0; x < cols ; x++) {

            for (y = 0 ; y < rows ; y++) {

                PlayField[x][y] = Color.black;

            }

        }

    }



    public int GetRandomShapeNr() {

        int ShapeNr;

        do {

            ShapeNr = (int) (Math.random() * ShapeSet.size());

        } while (ShapeNr >= ShapeSet.size());

        return ShapeNr;

    }



    public void GetNextRandomShape() {

        if (FallingShape == null) {

                FallingShape = (Shape) ShapeSet.elementAt(GetRandomShapeNr());

        } else {

                FallingShape = NextShape;

        }

        FallingShape.Init();

        if (!FallingShape.CheckIfShapeFits(PlayField, 0, 0, false)) {

            GameOver();

        }

        NextShape = (Shape) ShapeSet.elementAt(GetRandomShapeNr());

        Statistics.DisplayNextShape(NextShape);

    }



    public void GameOver() {

        play (SOUND_GAMEOVER);

        myPlayFieldCanvas.GameOver();

        Statistics.GameOver();

        InitNewGame();

    }



    public void start() {

        if(killme == null) {

            killme = new Thread(this);

            killme.start();

        }

    }



    public void stop() {

        killme = null;

    }



    public void run() {

        while (killme != null) {

            try {

                Thread.sleep(Statistics.GetGameSpeed());

            } catch (InterruptedException e){}

            if (!GamePaused) {

                if (FallingShape != null) {

                    if (FallingShape.CheckIfShapeFits(PlayField, 0, 1, false)) {

                        ChangeShapePosition(0, 1, false);

                    } else {

                        play (SOUND_DROP);

                        FallingShape.PlaceInPlayField(PlayField);

                        myPlayFieldCanvas.RepaintPlayField();

                        Statistics.AddScore(FallingShape.GetValue());

                        CheckForFullLines();

                        GetNextRandomShape();

                    }

                }

                myPlayFieldCanvas.repaint(FallingShape);

            }

        }

        killme = null;

    }



    public void CheckForFullLines() {

        int Lines = 0;

        int x, y, yc;

        boolean Gap;

        for (y = 0; y < rows; y++) {

            Gap = false;

            for (x = 0; x < cols; x++) {

                if (PlayField[x][y] == Color.black) Gap = true;

            }

            if (!Gap) {

                Lines++;

                for (yc = y - 1; yc >= 0; yc--) {

                    for (x = 0; x < cols; x++) {

                        PlayField[x][yc + 1] = PlayField[x][yc];

                    }

                }

                for (x = 0; x < cols; x++) {    // Delete top row.

                    PlayField[x][0] = Color.black;

                }

            }

        }

        if (Lines > 0) {

            play (SOUND_LINE);

            myPlayFieldCanvas.RepaintPlayField();

        }

        Statistics.AddLines(Lines);

    }



    public boolean action(Event evt, Object arg) {

        if ("About".equals(arg)) {

            GamePaused = true;

            myPlayFieldCanvas.About();

        }

        if ("Pause / Resume".equals(arg)) {

            if (GamePaused) {

                myPlayFieldCanvas.ShowAboutBox = false;

                myPlayFieldCanvas.RepaintPlayField();

            }



            GamePaused = !GamePaused;

        }

        return true;

    }



    public synchronized boolean handleEvent(Event e) {

        int xChange = 0;

        int yChange = 0;

        boolean Rotate = false;



        switch (e.id) {

        case Event.ACTION_EVENT:

            return action(e, e.arg);

        case Event.KEY_ACTION:

        case Event.KEY_PRESS:

            switch (e.key) {

            case 'b':

            case 'B':           // Move block to left

                xChange--;

                break;

            case 'm':

            case 'M':           // Move block to right

                xChange++;

                break;

            case ' ':           // Move block down

                yChange++;

                break;

            case 'n':

            case 'N':           // Rotate block

                Rotate = true;

                break;

            default:

                return false;

            }

            break;

        default:

            return false;

        }

        ChangeShapePosition (xChange, yChange, Rotate);

        return true;

    }



    public void ChangeShapePosition(int xChange, int yChange, boolean Rotate) {

        while (!FallingShape.IsReady()) ;

        if (FallingShape.CheckIfShapeFits(PlayField, xChange, yChange, Rotate)) {

            FallingShape.ChangePosition(xChange, yChange, Rotate);

            myPlayFieldCanvas.repaint(FallingShape);

        }

    }

}



class StatisticsCanvas extends Canvas implements ImageObserver {

    public static final Color textColor = Color.black;

    public static final int MaxLevel = 9;

    public static final int myFontHeight = 16;



    protected Font BlocksFont;

    protected FontMetrics BlocksFontMetrics;

    protected int Level, Lines, Score;

    protected Shape NextShape = null;



    public void InitNewGame() {

        Level = Lines = Score = 0;

    }



    public void GameOver() {



    }



    public int GetGameSpeed() {

        switch (Level) {

            case 0:     return 700;

            case 1:     return 600;

            case 2:     return 500;

            case 3:     return 400;

            case 4:     return 350;

            case 5:     return 300;

            case 6:     return 250;

            case 7:     return 200;

            case 8:     return 150;

            case 9:     return 100;

            default:    return 100;

        }

    }



    public StatisticsCanvas() {

        reshape(0, 0, 100, 100);

        BlocksFont = new Font("TimesRoman",Font.PLAIN,20);

        setFont(BlocksFont);

        BlocksFontMetrics = getFontMetrics(BlocksFont);

        InitNewGame();

    }



    public void AddScore(int s) {

        Score += s;

        repaint();

    }



    public void AddLines(int ln) {

        switch (ln) {

            case 1:

                AddScore (10);

                break;

            case 2:

                AddScore (20);

                break;

            case 3:

                AddScore (30);

                break;

            case 4:

                AddScore (40);

                break;

        }

        Lines += ln;

        if (Lines > (10 * (Level + 1))) AddLevel();

        repaint();

    }



    public void AddLevel() {

        Blocks.play (Blocks.SOUND_NEXTLEVEL);

        if (Level < MaxLevel) Level++;

        repaint();

    }



    public void DisplayNextShape(Shape s) {

        NextShape =  s;

        repaint();

    }



    public void paint(Graphics g) {

        g.setColor(textColor);

        g.drawString("Level: " + Level, 0, myFontHeight);

        g.drawString("Lines: " + Lines, 0, myFontHeight * 3);

        g.drawString("Score: " + Score, 0, myFontHeight * 5);

        g.drawString("Next:", 0, myFontHeight * 7);

        if (NextShape != null) {

            NextShape.DisplayAbs(g, 10, myFontHeight * 7 + 10);

        }

    }

}



class PlayFieldCanvas extends Canvas implements ImageObserver {

    public static final int fh1 = 17, fh2 = 17;

    public static final int BorderWidth = 5;

    public static final Color BorderColor = Color.blue;

    protected Shape FallingShape = null;

    boolean FallingShapeNeedRepaint = false;

    boolean PlayFieldNeedRepaint = false;

    boolean DiscardGame = false;



    boolean ShowAboutBox = false;

    protected Font BlocksFont1, BlocksFont2;





    Color PlayField[][] = null;



    public void About() {

        ShowAboutBox = true;

        repaint();

    }



    public void GameOver() {

        DiscardGame = true;

        repaint();

    }



    public void RepaintPlayField() {

        PlayFieldNeedRepaint = true;

        repaint();

    }



    public void SetPlayField(Color pv[][]) {

            PlayField = pv;

    }



    public PlayFieldCanvas() {

        BlocksFont1 = new Font("TimesRoman",Font.BOLD,20);

        FontMetrics BlocksFontMetrics1 = getFontMetrics(BlocksFont1);

        BlocksFont2 = new Font("TimesRoman",Font.PLAIN,14);

        FontMetrics BlocksFontMetrics2 = getFontMetrics(BlocksFont2);

        reshape(0, 0, Blocks.ElementSize * Blocks.cols + BorderWidth * 2, Blocks.ElementSize * Blocks.rows + BorderWidth );

    }



    public void repaint(Shape Shp) {

       FallingShape = Shp;

        FallingShapeNeedRepaint = true;

        repaint();

    }



    public void DrawLines (Graphics g, int y1, int y2) {

        for (int y = y1 * Blocks.ElementSize; y < y2 * Blocks.ElementSize; y++) {

            g.drawLine (BorderWidth, y, BorderWidth + Blocks.cols * Blocks.ElementSize, y);

        }

    }



    public void GraphicsEffect(Graphics g, int y1, int y2) {

        for (int l = 0; l < 10; l++) {

            g.setColor(Color.red);

            DrawLines (g, y1, y2);

            g.setColor(Color.green);

            DrawLines (g, y1, y2);

            g.setColor(Color.blue);

            DrawLines (g, y1, y2);

            g.setColor(Color.black);

            DrawLines (g, y1, y2);

        }

    }



    public void DiscardIt(Graphics g) {

        DiscardGame = false;

        GraphicsEffect (g, 0, Blocks.rows);

    }



    public void DisplayAboutBox(Graphics g)

    {

        int y = 1;

        // Clear background

        g.setColor(Color.black);

        g.fillRect (BorderWidth, 0, Blocks.ElementSize * Blocks.cols, Blocks.ElementSize * Blocks.rows);

        g.setFont(BlocksFont1);

        g.setColor(Color.red);

        g.drawString("BLOCKS", 20, fh1);

        g.setFont(BlocksFont2);

        g.setColor(Color.cyan);

        g.drawString("v 1.01", BorderWidth + 110, fh1);

        g.drawString("Copyright (c) 1995, 1996", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("Iwan van Rienen", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("This program is postcard", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("ware. If you like this", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("program buy a stamp and", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("a postcard and send it to: ", BorderWidth + 2, fh1 + fh2 * y++);        g.setColor(Color.green);

        g.setColor(Color.yellow);

        g.drawString("Iwan van Rienen", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("J. Maetsuyckerstr. 145", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("2593 ZG  The Hague", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("The Netherlands", BorderWidth + 2, fh1 + fh2 * y++);

        g.setColor(Color.cyan);

        g.drawString("The JAVA source is free", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("Visit my homepage at", BorderWidth + 2, fh1 + fh2 * y++);

        g.setColor(Color.green);

        g.drawString("http://www.bart.nl/~ivr", BorderWidth + 2, fh1 + fh2 * y++);

        g.drawString("E-mail to ivr@bart.nl", BorderWidth + 2, fh1 + fh2 * y++);

    }



    public void update(Graphics g) {

        if (DiscardGame) {

            DiscardIt(g);

        }

        if (PlayFieldNeedRepaint) {

            DrawPlayField(g);

        }

        DrawFallingShape(g);

        if (ShowAboutBox) {

            DisplayAboutBox(g);

        }

    }



    public void paint(Graphics g) {

        if (ShowAboutBox) {

            DisplayAboutBox(g);

        } else {

            DrawPlayField(g);

            FallingShapeNeedRepaint = true;

            DrawFallingShape(g);

        }

    }



    public void DrawFallingShape(Graphics g) {

        if (FallingShapeNeedRepaint && FallingShape != null) {

            FallingShape.hide(g, BorderWidth);

            while (!FallingShape.IsReady()) ;

            FallingShape.Display(g, BorderWidth);

            FallingShapeNeedRepaint = false;

        }

    }



    public void DrawPlayField(Graphics g) {

        int x, y;

        g.setColor(BorderColor);

        // Draw left border

        g.fillRect (0, 0, BorderWidth, Blocks.ElementSize * Blocks.rows);

        // Draw right border

        g.fillRect (Blocks.ElementSize * Blocks.cols + BorderWidth, 0, BorderWidth, Blocks.ElementSize * Blocks.rows);

        // Draw bottom border

        g.fillRect (0, Blocks.ElementSize * Blocks.rows, Blocks.ElementSize * Blocks.cols + BorderWidth * 2, BorderWidth);

        for (x = 0; x < Blocks.cols; x++) {

            for (y = 0; y < Blocks.rows; y++) {

                if (PlayField[x][y] != Color.black) {

                    g.setColor(PlayField[x][y]);

                    g.fillRect(BorderWidth + x * Blocks.ElementSize + 1, y * Blocks.ElementSize + 1, Blocks.ElementSize - 2, Blocks.ElementSize - 2);

                    g.setColor(Color.white);

                    g.drawRect(BorderWidth + x * Blocks.ElementSize, y * Blocks.ElementSize, Blocks.ElementSize - 1, Blocks.ElementSize - 1);

                } else {

                    g.setColor(Color.black);

                    g.fillRect(BorderWidth + x * Blocks.ElementSize, y * Blocks.ElementSize, Blocks.ElementSize, Blocks.ElementSize);

                }

            }

        }

    PlayFieldNeedRepaint = false;

    }

}



class Element {

    protected int x, y;

    int oldX, oldY;

    protected int xInShape, yInShape;

    protected int OriginalX, OriginalY;

    protected int OriginalXInShape, OriginalYInShape;

    protected Color clr;

    protected boolean ErasePossible;



    public Element(int xPos, int yPos, Color c) {

        ErasePossible = false;

        xInShape = OriginalXInShape = xPos;

        yInShape = OriginalYInShape = yPos;

        x = OriginalX = xPos + Blocks.cols / 2 - (Blocks.MaxElement + 1)/ 2;

        y = OriginalY = yPos;

        clr = c;

    }



    public void Init() {

        ErasePossible = false;

        x = OriginalX; y = OriginalY;

        xInShape = OriginalXInShape;

        yInShape = OriginalYInShape;

    }



    public void hide (Graphics g, int xOffs, int yOffs) {

        if (ErasePossible) {

            int Size = Blocks.ElementSize;

            g.setColor(Color.black);

            g.fillRect(xOffs + oldX * Size, yOffs + oldY * Size, Size, Size);

            ErasePossible = false;

        }

    }



    public void Display (Graphics g, int xOffs, int yOffs) {

        int Size = Blocks.ElementSize;

        g.setColor(clr);

        g.fillRect(xOffs + x * Size + 1, yOffs + y * Size + 1, Size - 2, Size - 2);

        g.setColor(Color.white);

        g.drawRect(xOffs + x * Size, yOffs + y * Size, Size - 1, Size - 1);

        oldX = x;

        oldY = y;

        ErasePossible = true;

    }



     public void DisplayAbs (Graphics g, int xOffs, int yOffs) {

        int Size = Blocks.ElementSize;

        g.setColor(clr);

        g.fillRect(xOffs + OriginalXInShape * Size + 1, yOffs + OriginalYInShape * Size + 1, Size - 2, Size - 2);

        g.setColor(Color.white);

        g.drawRect(xOffs + OriginalXInShape * Size, yOffs + OriginalYInShape * Size, Size - 1, Size - 1);

	}



    public boolean CheckIfElementFits(Color PlayField[][],

                                      int xOffs, int yOffs, boolean Rotate) {

       if (Rotate) {

            xOffs += Blocks.MaxElement - yInShape - xInShape;

            yOffs += xInShape - yInShape;

        }

        if (x + xOffs < 0 || x + xOffs >= Blocks.cols) return false;

        if (y + yOffs >= Blocks.rows ) return false;

        if (PlayField[x + xOffs][y + yOffs] != Color.black) return false;

        return true;

    }



    public void ChangeElementPosition(int xOffs, int yOffs, boolean Rotate) {

        if (Rotate) {

            xOffs += Blocks.MaxElement - yInShape - xInShape;

            yOffs += xInShape - yInShape;

            int tempxInShape = xInShape;

            xInShape = Blocks.MaxElement - yInShape;

            yInShape = tempxInShape;

        }

        x += xOffs;

        y += yOffs;

    }



    public int GetXPos() { return x; }

    public int GetYPos() { return y; }

    public Color GetColor() { return clr; }



}



class Shape {

    protected Vector Elements;

    protected int Value;

    protected boolean DrawReady = true;



    public Shape() {

        DrawReady = true;

    }



    public void Init() {

        DrawReady = true;

        Element WalkElement;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement = (Element) Elements.elementAt(ix);

            WalkElement.Init();

        }

   }



    public Shape(int a, int b, int c, int d, Color clr, int v) {

        Value = v;

        Elements = new Vector();

        AddElements(0, a, clr);

        AddElements(1, b, clr);

        AddElements(2, c, clr);

        AddElements(3, d, clr);

        Init();

    }



    protected void AddElements (int row, int a, Color clr) {

        if ((a & 0xf000) > 0) Elements.addElement (new Element(0, row, clr));

        if ((a & 0x0f00) > 0) Elements.addElement (new Element(1, row, clr));

        if ((a & 0x00f0) > 0) Elements.addElement (new Element(2, row, clr));

        if ((a & 0x000f) > 0) Elements.addElement (new Element(3, row, clr));

	}



    public void hide (Graphics g, int xOffs) {

        Element WalkElement;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement = (Element) Elements.elementAt(ix);

            WalkElement.hide(g, xOffs, 0);

        }

    }



    public void Display (Graphics g, int xOffs) {

        Element WalkElement;

        DrawReady = false;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement = (Element) Elements.elementAt(ix);

            WalkElement.Display(g, xOffs, 0);

		}

        DrawReady = true;

    }



   public void DisplayAbs (Graphics g, int xAbs, int yAbs) {

        Element WalkElement;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement = (Element) Elements.elementAt(ix);

            WalkElement.DisplayAbs(g, xAbs, yAbs);

		}

	}



    public boolean CheckIfShapeFits(Color PlayField[][], int xOffs, int yOffs,

                                    boolean Rotate) {

        Element WalkElement;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement= (Element) Elements.elementAt(ix);

            if (!WalkElement.CheckIfElementFits

                   (PlayField, xOffs, yOffs, Rotate)) return false;

        }

        return true;

    }



    public void ChangePosition(int xOffs, int yOffs, boolean Rotate) {

        Element WalkElement;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement = (Element) Elements.elementAt(ix);

            WalkElement.ChangeElementPosition(xOffs, yOffs, Rotate);

        }

    }



    public void PlaceInPlayField(Color PlayField[][]) {

        Element WalkElement;

        for (int ix = 0; ix < Elements.size(); ix++) {

            WalkElement= (Element) Elements.elementAt(ix);

            PlayField[WalkElement.GetXPos()]

                     [WalkElement.GetYPos()] = WalkElement.GetColor();

        }

    }



    public int GetValue() { return Value; }

    public boolean IsReady() { return DrawReady; }





}