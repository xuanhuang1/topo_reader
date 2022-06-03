#!/usr/bin/env python
#####################################################################################
############################   PARAMETERS   #########################################
#####################################################################################
basefile        = "flexgui"
#####################################################################################

#####################################################################################
############  LIBRAREIS NEEDED ######################################################
#####################################################################################
from Tkinter import *
import os
import os.path
import sys
import time
import re
#####################################################################################
                             
scr1 = os.path.getmtime(basefile + '_in.txt')
    
class Hello(Frame):
    arctype = 0
    arccolor = 0
    
    def __init__(self, master=None):
        self.master = master
        Frame.__init__(self, master)
        self.myframe = Frame(self.master)
        self.myframe.pack()

        self.gpersv = IntVar()
        self.widgets = {}
        self.wvars   = {}
        self.sliders = {}
        self.svars = {}
        self.flags = {}
        self.fvars = {}
        self.idle_func = self.myidle
        self.after(1,self.idle)
        self.pack()
        self.make_widgets()
        self.readscript()
        #print("done initializing")
        

    def printscript(self):
        global basefile
        fs = open(basefile +'_in.txt')
        scr = fs.read()

        # remove all comments
        scr = re.sub(r'\%.*?\n', '', scr)

        # REPLACE EACH WIDGET WITH THE OUTPUT VALUE
        for x in self.descr_widgets :
            if x in self.wvars :
                #print x ," => ",str(self.wvars[x].get())
                scr = scr.replace(x,str(self.wvars[x].get()),1)
            else:
                print x ," NOT FOUND "

        fo = open(basefile +'_out.txt', 'w')
        fo.write(scr)
        #print scr

    def createwidgets(self):
        ###################################################
        #clear widgets if any
        for s in self.widgets.values():
            s.pack_forget()
            s.destroy()
        self.widgets = {}
        self.wvars = {}
        self.myframe.destroy()
        self.myframe = Frame(self.master)
        self.myframe.pack()


        ##################################################
        # Create the actual widgets
        for x in self.descr_widgets :
            #print "________________________________________"
            #print x
            parameters = re.split('{|}|=|:|\(|\)|,',re.sub(' ', '', x))
            #print parameters , parameters[1]
            widget_type = parameters[1]
            ##########################################################
            ############################### PARSE A SLIDER ###########
            if widget_type == "SLIDER" :
                #print ""
                name   = parameters[2]
                init_l = eval(parameters[3])
                init_u = eval(parameters[4])
                init_s = eval(parameters[6])
                init_i = eval(parameters[8])
                v = DoubleVar()
                b = Scale(self.myframe, label=name, from_=init_l, to=init_u, resolution=init_s, \
                          orient=HORIZONTAL, tickinterval=(init_u - init_l)/4.0, variable=v, \
                          length=520, \
                          command= lambda sc : self.printscript())
                b.set(init_i)
                b.pack(expand=True, fill=X, side=TOP)
                self.wvars  [x] = v
                self.widgets[x] = b
            ##########################################################
            ############################### PARSE A FLAG #############
            elif widget_type == "FLAG" :
                #print "DDDD"
                name   = parameters[2]
                v = IntVar()
                #\v.set(int(parametersp-2]))
                v.set(int(parameters[-2]))
                b = Checkbutton(self.myframe, text=name, variable=v, command= self.printscript) 
                b.pack(expand=True, fill=X, side=TOP)
                self.wvars  [x] = v
                self.widgets[x] = b

            ##########################################################
            ############################### PARSE A MULTI #############
            elif widget_type == "MULTI" :
                name   = parameters[2]
                b = Frame(self.myframe) 
                b.pack(expand=True, fill=X, side=TOP)
                v = StringVar()
                v.set(parameters[-2]) # initialize

                b1 = Label(b, text=name+": ")
                b1.pack(anchor=W,side=LEFT)
                VAL =0
                for text in parameters[3:-3]:
                    b1 = Radiobutton(b, text=text, variable=v, value=str(VAL),command= self.printscript)
                    b1.pack(anchor=W,side=LEFT)
                    VAL = VAL+1
                self.wvars  [x] = v
                self.widgets[x] = b
            elif widget_type == "BO" :
                print "CCCC"
            elif widget_type == "SLIDER" :
                print "BBBB" 
            else :
                print "UNKNOWN WIDGET TYPE"



     ####################################################
    # PARSE INPUT SCRIPT
    def readscript(self):
        global basefile
        fs = open(basefile +'_in.txt')
        scr = fs.read()

        #remove spaces
        scr = re.sub(r'\%.*?\n', '', scr)

        #collect infomration about widgets
        self.descr_widgets = re.findall(r'{.+?}', scr)
        #print self.widgets
                
        self.createwidgets()
        
    def myidle(self):
        #print("i am idle")
        global scr1
        global basefile
                
        scr2 = os.path.getmtime(basefile+'_in.txt')
        #print "TIME = ",scr2
        if scr1 != scr2 :
            #print("script file changed")
            self.readscript()
            scr1 = scr2
            self.printscript()

        time.sleep(0.030)
        
    def idle(self):
        #print("after-idle")
        self.idle_func()
        self.after(1,self.idle)

    def quit(self):
        self.myquit()
        
    def myquit(self):
        Frame.quit(self)

    def make_widgets(self):
        widget = Button(self, text = "Quit", command=self.myquit)
        widget.pack(expand=True, fill=X,side=TOP)

        
###################################################################
        ##########################################################
###################################################################


from Tkinter import Frame


class Scrolledframe(Frame):

        def cget(self,item):
                return Frame.cget(self,item) if not hasattr(self,item) else getattr(self,item)
        __getitem__ = cget

        def configure(self,cnf=None,**kw):
                if kw:cnf=Tkinter._cnfmerge((cnf,kw))
                for key in cnf.keys():
                        Frame.configure(self,cnf) if not hasattr(self,key) else setattr(self,key,cnf[key])
        def __setitem__(self,item,value):self.configure({item:value})

        def __call__(self):
                return self.client

        def __init__(self,master=None,stretch=True,cnf={},**kw):
                Frame.__init__(self,master,cnf,**kw)
                self.client=Frame(self,border=0)

                # width and height of Scrolledframe
                self.W=1.0
                self.H=1.0
                # top left corner coordinates of client frame
                self.client_x=0
                self.client_y=0
                # width and height of client frame
                self.client_w=1.0
                self.client_h=1.0
                # scrollcommands (default)
                self.xscrollcommand=lambda *args:None
                self.yscrollcommand=lambda *args:None
                # scrollincrements
                self.xscrollincrement=5
                self.yscrollincrement=5
                # stretches
                self.stretch=stretch
                self.stretch_x=stretch
                self.stretch_y=stretch

                self.bind("<Expose>",self.update_scrollregion)

        def xview(self,event,value,units='pages'):
                if event == "moveto":
                        fraction=float(value)
                        if fraction <= 0.0:self.client_x=0
                        elif fraction >= float(self.client_w-self.W)/self.client_w:self.client_x=self.W-self.client_w
                        else:self.client_x=int(-self.client_w*fraction)
                elif event == "scroll":
                        amount=int(value)
                        if self.client_x == 0 and amount < 0:return
                        if self.client_x == self.W-self.client_w and amount > 0:return
                        self.client_x=self.client_x-(amount*self.xscrollincrement if units == "units" else amount*self.W*0.99)
                else:return

                self.update_scrollx()
                self.client.place_configure(x=self.client_x)

        def yview(self,event,value,units='pages'):
                if event == "moveto":
                        fraction=float(value)
                        if fraction <= 0.0:self.client_y=0
                        elif fraction >= float(self.client_h-self.H)/self.client_h:self.client_y=self.H-self.client_h
                        else:self.client_y=int(-self.client_h*fraction)
                elif event == "scroll":
                        amount=int(value)
                        if self.client_y == 0 and amount < 0:return
                        if self.client_x == self.H-self.client_h and amount > 0:return
                        self.client_y=self.client_y-(amount*self.yscrollincrement if units == "units" else amount*self.H)
                else:return

                self.update_scrolly()
                self.client.place_configure(y=self.client_y)

        def update_scrollx(self,*args):
                low=0.0 if self.client_x >= 0 else -float(self.client_x)/self.client_w
                high=1.0 if self.client_x+self.client_w <= self.W else low+float(self.W)/self.client_w
                if low <= 0.0:
                        self.client_x=0
                elif high >= 1.0:
                        self.client_x=self.W-self.client_w
                        low=-float(self.client_x)/self.client_w
                self.stretch_x=self.stretch if (self.client_w < self.W) else False
                self.xscrollcommand(low,high)

        def update_scrolly(self,*args):
                low=0.0 if self.client_y >= 0 else -float(self.client_y)/self.client_h
                high=1.0 if self.client_y+self.client_h <= self.H else low+float(self.H)/self.client_h
                if low <= 0.0:
                        self.client_y=0
                elif high >= 1.0:
                        self.client_y=self.H-self.client_h
                        low=-float(self.client_y)/self.client_h
                self.stretch_y=self.stretch if (self.client_h < self.H) else False
                self.yscrollcommand(low,high)

        def update_scrollregion(self,*args):
                if len(self.client.children):
                        self.client_w=self.client.winfo_reqwidth()
                        self.client_h=self.client.winfo_reqheight()
                        self.W=self.winfo_width()
                        self.H=self.winfo_height()

                        self.update_scrolly()
                        self.update_scrollx()

                        self.client.place_configure(
                                anchor="nw",
                                y=self.client_y,height=self.H if self.stretch_y else self.client_h,
                                x=self.client_x,width=self.W if self.stretch_x else self.client_w
                        )
                else:
                        self.xscrollcommand(0.0,1.0)
                        self.yscrollcommand(0.0,1.0)
                        self.client.place_forget()

if __name__ == "__main__":
        from Tkinter import Label,Tk,Scrollbar,Checkbutton,BooleanVar
        import Tkinter
        root=Tk(None)
        root.geometry("%dx%d%+d%+d" % (600, 800, 0, 0))
        root.title("Flexible GUI")
        root.columnconfigure(0,weight=1)
        root.rowconfigure(0,weight=1)

        #root = Tkinter.Frame(root,width=1768, height=1576, bg="", colormap="new")
        #root = Tkinter.Frame(root, bg="", colormap="new")
        #root.pack()

        scrollframe=Scrolledframe(root,stretch=True)

        scrollx=Scrollbar(root,orient="horizontal",command=scrollframe.xview)
        scrollx.grid(row=1,column=0,sticky="nwse")
        scrolly=Scrollbar(root,orient="vertical",command=scrollframe.yview)
        scrolly.grid(row=0,column=1,sticky="nwse")
        scrollframe["xscrollcommand"]=scrollx.set
        scrollframe["yscrollcommand"]=scrolly.set

        scrollframe.grid(row=0,column=0,sticky="nwse")

        checkvar=BooleanVar(root,value=scrollframe["stretch"])
        def set_stretch():
                scrollframe["stretch"]=True if checkvar.get() else False
                scrollframe.update_scrollregion()

        stretch=Checkbutton(root,anchor="w",onvalue=True,offvalue=False,variable=checkvar,text="Center buttons",command=set_stretch)
        stretch.grid(sticky="nwse",padx=2,pady=2)


        hw = Hello(scrollframe())

        root.mainloop()



