# Generated by Django 2.1.3 on 2019-07-15 19:48

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('podconnect', '0004_auto_20190713_1727'),
    ]

    operations = [
        migrations.RenameField(
            model_name='adcdata',
            old_name='data',
            new_name='data_0',
        ),
        migrations.AddField(
            model_name='adcdata',
            name='data_1',
            field=models.IntegerField(default=0),
        ),
        migrations.AddField(
            model_name='adcdata',
            name='data_2',
            field=models.IntegerField(default=0),
        ),
        migrations.AddField(
            model_name='adcdata',
            name='data_3',
            field=models.IntegerField(default=0),
        ),
        migrations.AddField(
            model_name='adcdata',
            name='data_4',
            field=models.IntegerField(default=0),
        ),
        migrations.AddField(
            model_name='adcdata',
            name='data_5',
            field=models.IntegerField(default=0),
        ),
        migrations.AddField(
            model_name='adcdata',
            name='data_6',
            field=models.IntegerField(default=0),
        ),
    ]
