using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Diagnostics.Eventing.Reader;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;


namespace Formula_Reaction_Time_V3
{
    public partial class Form1 : Form
    {
        clsResize _form_resize;
        Random rnd = new Random();
        Stopwatch timer = new Stopwatch();
        public static Form1 f1Init;
        public int rnd_time_tolerance_down = 50;
        public int rnd_time_tolerance_up = 200;

        int score1 = 0;
        int score2 = 0;

        int waittime = 0;
        int winnertime = 0;

        int debounce_time1 = 0;
        int debounce_time2 = 0;
        bool unlock_button1 = true;
        bool unlock_button2 = true;

        static int starting_time_sequence = 0;
        static int rnd_time = 0;
        static int random_timer_ticks = 0;
        bool ready1 = false;
        bool ready2 = false;
        bool block = false;
        bool game = false;
        bool game_OK = false;
        int time1 = 0;
        int time2 = 0;
        bool player1_finish = false;
        bool player2_finish = false;

        public Form1()
        {
            InitializeComponent();
            f1Init = this;
            _form_resize = new clsResize(this);
            this.Load += _Load;
            this.Resize += _Resize;
            this.BackgroundImage = pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.p;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Reset_Game();
            MegaReset();
        }

        private void MegaReset()
        {
            textBox1.Text = "0";
            textBox2.Text = "0";
            score1 = 0;
            score2 = 0;
        }

        private void Reset_Game()
        {           
            textBox3.Text = "0";
            textBox4.Text = "0";
            textBox5.Text = "Ready?";
            textBox6.Text = "Ready?";
            pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.grey;
            textBox7.Visible = false;
            starting_time_sequence = 0;
            rnd_time = 0;
            random_timer_ticks = 0;
            ready1 = false;
            ready2 = false;
            block = false;
            game = false;
            game_OK = false;
            time1 = 0;
            time2 = 0;
            player1_finish = false;
            player2_finish = false;
            start_sequence.Stop();
            random_timer.Stop();
            wait.Stop();
            winner.Stop();
            d1.Stop();
            d2.Stop();
            debounce_time1 = 0;
            debounce_time2 = 0;
            unlock_button1 = true;
            unlock_button2 = true;
            textBox7.Visible = false;
            textBox7.Text = "Game started";
            timer.Stop();
            timer.Reset();
        }



        #region Form Resizing - změna velikosti okna        
        private void _Load(object sender, EventArgs e)
        {
            _form_resize._get_initial_size();
        }

        private void _Resize(object sender, EventArgs e)
        {
            _form_resize._resize();
        }
        #endregion       

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.R)
            {
                Reset_Game();
                MegaReset();
            }
            if (!block) 
            {
                if (e.KeyCode == Keys.F11 && unlock_button1)
                {
                    d1.Start();
                    unlock_button1 = false;

                    if (game)
                    {
                        if (game_OK)
                        {
                            time1 = (int)timer.ElapsedMilliseconds;
                            player1_finish = true;
                            textBox5.Text = "OK";
                            textBox7.Visible = false;
                            textBox3.Text = time1.ToString();
                        }
                        else
                        {
                            timer.Stop();
                            start_sequence.Stop();
                            random_timer.Stop();
                            block = true;
                            pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.orange;
                            textBox5.Text = "Fail";
                            if (score1 > 0)
                            {
                                score1--;
                            }
                            textBox1.Text = score1.ToString();
                            textBox2.Text = score2.ToString();
                            wait.Start();
                        }
                    }
                    else if (!ready1)
                    {
                        ready1 = true;
                        textBox5.Text = "READY";
                    }
                }
                if (e.KeyCode == Keys.F12 && unlock_button2)
                {
                    d2.Start();
                    unlock_button2 = false;

                    if (game)
                    {
                        if (game_OK)
                        {
                            time2 = (int)timer.ElapsedMilliseconds;
                            player2_finish = true;
                            textBox6.Text = "OK";
                            textBox7.Visible = false;
                            textBox4.Text = time2.ToString();
                        }
                        else
                        {
                            timer.Stop();
                            start_sequence.Stop();
                            random_timer.Stop();
                            block = true;
                            pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.orange;
                            textBox6.Text = "Fail";
                            if (score2 > 0)
                            {
                                score2--;
                            }
                            textBox1.Text = score1.ToString();
                            textBox2.Text = score2.ToString();
                            wait.Start();
                        }
                    }
                    else if (!ready2)
                    {
                        ready2 = true;
                        textBox6.Text = "READY";
                    }                   
                }

                if (ready1 && ready2 && !game)
                {
                    block = true;
                    game = true;
                    textBox7.Visible = true;
                    pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.green;
                    start_sequence.Start();             
                }

                if (player1_finish && player2_finish)
                {
                    if (time1 < time2)
                    {
                        score1++;
                    }
                    else if (time1 > time2)
                    {
                        score2++;
                    }
                    else
                    {
                        score1++;
                        score2++;
                    }
                    timer.Stop();
                    block = true;
                    textBox1.Text = score1.ToString();
                    textBox2.Text = score2.ToString();
                    if (score1 == 5 && score2 != 5)
                    {
                        textBox5.Text = "WINNER";
                        winner.Start();
                    }
                    else if (score2 == 5 && score1 != 5)
                    {
                        textBox6.Text = "WINNER";
                        winner.Start();
                    }
                    else
                    {
                        wait.Start();
                    }                  
                }
            }
        }

        private void start_sequence_Tick(object sender, EventArgs e)
        {
            starting_time_sequence += 1;
            if (starting_time_sequence == 1)
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources._1;
            }
            if (starting_time_sequence == 2)
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources._2;
            }
            if (starting_time_sequence == 3)
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources._3;
                block = false;
                textBox6.Text = "";
                textBox5.Text = "";
                textBox7.Visible = false;
            }
            if (starting_time_sequence == 4)
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources._4;
            }
            if (starting_time_sequence == 5)
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.red;              
                starting_time_sequence = 0;
                rnd_time = rnd.Next(rnd_time_tolerance_down, rnd_time_tolerance_up);
                random_timer.Start();
                start_sequence.Stop();
            }
        }

        private void random_timer_Tick(object sender, EventArgs e)
        {
            random_timer_ticks += 1;
            if (random_timer_ticks == rnd_time)
            {
                random_timer_ticks = 0;
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.grey;
                timer.Start();
                random_timer.Stop();
                textBox7.Text = "Press button";
                textBox7.Visible = true;
                game_OK = true;
            }
        }

        private void wait_Tick(object sender, EventArgs e)
        {
            waittime++;
            if (waittime == 3)
            {
                block = true;
                waittime = 0;
                wait.Stop();
                Reset_Game();
            }
        }

        private void winner_Tick(object sender, EventArgs e)
        {
            winnertime++;
            if (winnertime % 2 == 0)
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.green;
            }
            else
            {
                pictureBox1.BackgroundImage = Formula_Reaction_Time_V3.Properties.Resources.grey;
            }
            if (winnertime == 50)
            {
                block = true;
                winnertime = 0;
                winner.Stop();
                Reset_Game();
                MegaReset();
            }
        }

        private void d1_Tick(object sender, EventArgs e)
        {           
            debounce_time1++;   

            if (debounce_time1 == 2)
            {
                debounce_time1 = 0;
                unlock_button1 = true;
                d1.Stop();
            }
        }

        private void d2_Tick(object sender, EventArgs e)
        {
            debounce_time2++;

            if (debounce_time2 == 2)
            {
                debounce_time2 = 0;
                unlock_button2 = true;
                d2.Stop();
            }
        }
    }
}
