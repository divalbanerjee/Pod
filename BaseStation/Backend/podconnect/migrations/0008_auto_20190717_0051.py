# Generated by Django 2.1.3 on 2019-07-17 00:51

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('podconnect', '0007_auto_20190716_2159'),
    ]

    operations = [
        migrations.AlterField(
            model_name='motiondata',
            name='relay_state_buff_0',
            field=models.CharField(default=0, max_length=1),
        ),
        migrations.AlterField(
            model_name='motiondata',
            name='relay_state_buff_1',
            field=models.CharField(default=0, max_length=1),
        ),
        migrations.AlterField(
            model_name='motiondata',
            name='relay_state_buff_2',
            field=models.CharField(default=0, max_length=1),
        ),
        migrations.AlterField(
            model_name='motiondata',
            name='relay_state_buff_3',
            field=models.CharField(default=0, max_length=1),
        ),
    ]
