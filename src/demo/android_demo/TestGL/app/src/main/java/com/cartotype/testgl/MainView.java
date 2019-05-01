package com.cartotype.testgl;

import android.content.Context;
import com.cartotype.*;

public class MainView extends MapView
    {
    MainView(Context aContext,Framework aFramework)
        {
        super(aContext,aFramework);
        m_framework = aFramework;
        }

    public void onTap(double aX,double aY)
        {
        m_framework.setPerspective(!m_framework.getPerspective());
        }

    private Framework m_framework;
    }
